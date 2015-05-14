#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Logger/Logger.hpp"
#include "Shader/Shader.hpp"
#include "Program/Program.hpp"
#include "Util/Util.hpp"
#include "MapLoader/MapLoader.hpp"
#include "Light/Light.hpp"
#include <thread>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <locale>
#include <chrono>
#include <algorithm>
#include <array>
#include <ctime>
#include <random>
#include "ext/stb/stb_image.h"
#include "ext/stb/stb_image_write.h"
#include "ext/json/src/json.hpp"

using json = nlohmann::json;

using block_id = uint8_t;
using coord_type = uint8_t;

constexpr const_vec<float> init_win_size(960.f, 540.f);
constexpr const_vec<float> render_size(3840.f, 2160.f);

//Specify amount of chunks
constexpr const_vec<int32_t> num_chunks(16, 16, 1);
// Specify chunk sizes, chunk_size_*  and chunk_total must be a power of 2.
constexpr const_vec<int32_t> chunk_size(16, 16, 256);
constexpr uint64_t chunk_total =chunk_size.x*chunk_size.y*chunk_size.z;
constexpr const_vec<int> chunk_grouping(4, 4, 1);

constexpr const_vec<int> block_offset(0, sizeof(coord_type), 2*sizeof(coord_type));

constexpr GLsizei components_per_vtx = 9;

// Camera struct
struct camera {
	glm::quat orientation;
	glm::vec3 position;

	void rotate(glm::vec3 axis, float angle) {
		glm::quat rot = glm::angleAxis(angle, axis);
		orientation = rot * orientation;
		orientation = normalize(orientation);
	}
	void strafe(float amount) {
		move({0.f, amount, 0.f});
	}
	void climb(float amount) {
		move({0.f, 0.f, amount});
	}
	void advance(float amount) {
		move({amount, 0.f, 0.f});
	}
	void move(glm::vec3 delta) {
		auto tmp = glm::mat4_cast(orientation);
		auto x = glm::vec3(tmp[0][2], tmp[1][2], tmp[2][2]);
		auto y = -glm::vec3(tmp[0][0], tmp[1][0], tmp[2][0]);
		auto z = -glm::vec3(tmp[0][1], tmp[1][1], tmp[2][1]);
		position += x*delta.x + y*delta.y + z*delta.z;
	}

	glm::mat4 get_view() {
		auto tmp = glm::mat4_cast(orientation);
		tmp = glm::translate(tmp, position);
		return tmp;
	}
};

// A block just contains its own coordinates
struct block{
	coord_type x, y, z;
	block(int x=0, int y=0, int z=0):x(x),y(y),z(z){}
};

// A chunk contains a static array of blocks
//  (each chunk has the same blocks, with different IDs)
// It also contains its texture ID and an array of block IDs (which go into the
//    textures)
struct chunk{
	static std::vector<block> offsets;
	glm::ivec3 position;
	std::vector<block_id> *IDs;
	GLenum texnum;
	GLuint texid;
	GLuint tex;
	GLuint buffer_geometry;
	GLuint primitive_count;
	GLuint vertex_count;
	GLuint component_count;
	GLuint vtx_array;
	bool draw;
};

struct chunk_group {
	std::vector<chunk*> chunks;
	GLuint merged_geometry;
	GLuint vtx_array;
};

std::vector<block> chunk::offsets;

GLuint framebuffer_display_color_texture;

constexpr float pi = 3.14159;

Logger<wchar_t> wlog{std::wcout};


int cleanup(int rtval, std::wstring extra=L"");
bool readfile(const char* filename, std::string &contents);
bool process_gl_errors();

int main()
{

	// Generate chunk_x*chunk_y chunks
	std::vector<std::vector<chunk>> chunks(num_chunks.x);
	for(auto &v : chunks) {
		v.resize(num_chunks.y);
		for(auto &c : v) {
			c = chunk();
		}
	}

	// Fill chunk offsets with.. their offsets
	chunk::offsets.resize(chunk_total);
	for(int z=0;z<chunk_size.z;++z) {
		for(int y=0;y<chunk_size.y;++y) {
			for(int x=0;x<chunk_size.x;++x) {
				chunk::offsets[z*chunk_size.x*chunk_size.y+y*chunk_size.x+x] = block{x,y,z};
			}
		}
	}

	using namespace std::literals::chrono_literals;

	wlog.log(L"Starting up.\n");
	wlog.log(L"Initializing GLFW.\n");

	if(!glfwInit())
		return cleanup(-1);

	glfwSetErrorCallback(
		[](int, const char* msg){
			wlog.log(std::wstring{msg, msg+std::strlen(msg)}+L"\n");
		}
	);

	// Create window with context params etc.
	wlog.log(L"Creating window.\n");
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow *win = glfwCreateWindow(
		init_win_size.x, init_win_size.y, "Voxelator!", nullptr, nullptr
	);

	// If window creation fails, exit.
	if(!win)
		return cleanup(-2);

	int win_size[2];
	int &win_size_x=win_size[0], &win_size_y=win_size[1];
	glfwGetWindowSize(win, &win_size_x, &win_size_y);

	glfwMakeContextCurrent(win);

	wlog.log(L"Initializing GLEW.\n");
	glewExperimental = GL_TRUE;
	if(glewInit())
		return cleanup(-3);

	process_gl_errors();

	wlog.log(
		L"Any errors produced directly after GLEW initialization "
		L"should be ignorable.\n"
	);

	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);


	wlog.log("Generating Vertex Array Object.\n");
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	wlog.log(L"Creating Shaders.\n");

	Shader shader_generate_vert;
	shader_generate_vert.load_file(GL_VERTEX_SHADER, "assets/shaders/generate/shader.vert");

	wlog.log(L"Creating generate geometry shader.\n");
	Shader shader_generate_geom;
	shader_generate_geom.load_file(GL_GEOMETRY_SHADER, "assets/shaders/generate/shader.geom");

	wlog.log(L"Creating and linking generate shader program.\n");

	Program generate_program;
	generate_program.attach(shader_generate_vert);
	generate_program.attach(shader_generate_geom);
	generate_program.transform_feedback_varyings({"gPos", "gTexcoords", "gNormal"});
	generate_program.link();


	wlog.log(L"Creating frustum_culling vertex shader.\n");
	Shader shader_frustum_culling_vert;
	shader_frustum_culling_vert.load_file(GL_VERTEX_SHADER, "assets/shaders/frustum_culling/shader.vert");

	wlog.log(L"Creating frustum_culling geometry shader.\n");
	Shader shader_frustum_culling_geom;
	shader_frustum_culling_geom.load_file(GL_GEOMETRY_SHADER, "assets/shaders/frustum_culling/shader.geom");

	wlog.log(L"Creating and linking frustum_culling shader program.\n");

	Program frustum_culling_program;
	frustum_culling_program.attach(shader_frustum_culling_vert);
	frustum_culling_program.attach(shader_frustum_culling_geom);
	frustum_culling_program.transform_feedback_varyings({"gPos"});
	frustum_culling_program.link();

	
	wlog.log(L"Creating render vertex shader.\n");
	Shader shader_render_vert;
	shader_render_vert.load_file(GL_VERTEX_SHADER, "assets/shaders/render/shader.vert");

	wlog.log(L"Creating render fragment shader.\n");
	Shader shader_render_frag;
	shader_render_frag.load_file(GL_FRAGMENT_SHADER, "assets/shaders/render/shader.frag");

	wlog.log(L"Creating and linking render shader program.\n");

	Program render_program;
	render_program.attach(shader_render_vert);
	render_program.attach(shader_render_frag);
	glBindFragDataLocation(render_program, 0, "outColor");
	glBindFragDataLocation(render_program, 1, "outNormal");
	render_program.link();


	wlog.log(L"Creating lighting vertex shader.\n");
	Shader shader_lighting_vert;
	shader_lighting_vert.load_file(GL_VERTEX_SHADER, "assets/shaders/lighting/shader.vert");

	wlog.log(L"Creating lighting fragment shader.\n");
	Shader shader_lighting_frag;
	shader_lighting_frag.load_file(GL_FRAGMENT_SHADER, "assets/shaders/lighting/shader.frag");


	wlog.log(L"Creating and linking lighting shader program.\n");

	Program lighting_program;
	lighting_program.attach(shader_lighting_vert);
	lighting_program.attach(shader_lighting_frag);
	glBindFragDataLocation(lighting_program, 0, "outCol");
	lighting_program.link();


	wlog.log(L"Creating display vertex shader.\n");
	Shader shader_display_vert;
	shader_display_vert.load_file(GL_VERTEX_SHADER, "assets/shaders/display/shader.vert");

	wlog.log(L"Creating display fragment shader.\n");
	Shader shader_display_frag;
	shader_display_frag.load_file(GL_FRAGMENT_SHADER, "assets/shaders/display/shader.frag");

	wlog.log(L"Creating and linking display shader program.\n");

	Program display_program;
	display_program.attach(shader_display_vert);
	display_program.attach(shader_display_frag);
	glBindFragDataLocation(display_program, 0, "color");
	display_program.link();

	process_gl_errors();

	wlog.log(L"Loading spritesheet.\n");
	glActiveTexture(GL_TEXTURE0);
	GLuint spritesheet_tex;
	glGenTextures(1, &spritesheet_tex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, spritesheet_tex);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 4);
	float col[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, col);
	int spritesheet_x, spritesheet_y, spritesheet_n;
	unsigned char *spritesheet_data = stbi_load(
		"assets/images/minecraft.png", &spritesheet_x, &spritesheet_y,
		&spritesheet_n, 4
	);
	wlog.log(
		L"Spritesheet size: {" + std::to_wstring(spritesheet_x) + 
		L", " + std::to_wstring(spritesheet_y) + L"}, " + 
		std::to_wstring(spritesheet_n) + L"cpp\n"
	);
	glm::vec2 spritesheet_size(spritesheet_x, spritesheet_y);
	glm::vec2 sprite_size(16, 16);
	glm::vec2 sprite_vec = spritesheet_size/sprite_size;
	GLsizei sprites = sprite_vec.x*sprite_vec.y;
	uint8_t *transformed_pixel_data = new uint8_t[sprites*16*16*4];
	for(int tex_y = 0; tex_y < sprite_vec.y; ++tex_y ) {
		for(int tex_x = 0; tex_x < sprite_vec.x; ++tex_x ) {
			for(int pix_y = 0; pix_y < sprite_size.y; ++pix_y)
				for(int pix_x = 0; pix_x < sprite_size.x; ++pix_x) {
					size_t tf_index = (tex_y*sprite_vec.x + tex_x)*sprite_size.x*sprite_size.y;
					tf_index += pix_y*sprite_size.x + pix_x;
					tf_index *= 4;

					size_t orig_index = (tex_y*sprite_size.y+pix_y)*(spritesheet_x);
					orig_index += tex_x*sprite_size.x+pix_x;
					orig_index *= 4;

					transformed_pixel_data[tf_index+0] = spritesheet_data[orig_index+0];
					transformed_pixel_data[tf_index+1] = spritesheet_data[orig_index+1];
					transformed_pixel_data[tf_index+2] = spritesheet_data[orig_index+2];
					transformed_pixel_data[tf_index+3] = spritesheet_data[orig_index+3];
				}
		}
	}
	glTexImage3D(
		GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, sprite_size.x, sprite_size.y, sprites, 0, 
		GL_RGBA, GL_UNSIGNED_BYTE, transformed_pixel_data
	);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(spritesheet_data);
	wlog.log(
		L"Sprite size: {"+ std::to_wstring(sprite_size.x) + 
		L"," + std::to_wstring(sprite_size.y) +L"}\n"
	);
	glm::vec2 sprite_size_normalized = sprite_size/spritesheet_size;
	wlog.log(
		L"Sprite size (normalized): {" + 
		std::to_wstring(sprite_size_normalized.x) + L"," +
		std::to_wstring(sprite_size_normalized.y) +L"}\n"
	);
	glm::ivec2 n_vec_sprites = spritesheet_size/sprite_size;
	int n_sprites = n_vec_sprites.x*n_vec_sprites.y;
	wlog.log(
		L"Number of sprites: {"+ std::to_wstring(n_vec_sprites.x) + L"," + 
		std::to_wstring(n_vec_sprites.y) +L"} = " + std::to_wstring(n_sprites) +
		L" \n"
	);

	process_gl_errors();

	wlog.log(L"Creating ");
	wlog.log(std::to_wstring(chunk_total), false);
	wlog.log(L" blocks.\n", false);

	wlog.log(L"Generating Vertex Buffer Object.\n");
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(
		GL_ARRAY_BUFFER, sizeof(*chunk::offsets.data())*chunk::offsets.size(),
		chunk::offsets.data(), GL_STATIC_DRAW
	);
	process_gl_errors();

	chunk empty_chunk;
	empty_chunk.buffer_geometry=-1;
	empty_chunk.component_count=-1;
	empty_chunk.primitive_count=-1;
	empty_chunk.vertex_count=-1;
	empty_chunk.vtx_array=-1;
	empty_chunk.IDs = new std::vector<block_id>(chunk_total);
	std::fill(empty_chunk.IDs->begin(), empty_chunk.IDs->end(), 0);
	empty_chunk.tex = 0;
	empty_chunk.texnum = GL_TEXTURE0;
	glActiveTexture(empty_chunk.texnum);
	glGenTextures(1, &empty_chunk.texid);
	glBindTexture(GL_TEXTURE_3D, empty_chunk.texid);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, col);
	glTexImage3D(
		GL_TEXTURE_3D, 0, GL_RED, chunk_size.x, chunk_size.y, 
		chunk_size.z, 0, GL_RED, GL_UNSIGNED_BYTE, 
		empty_chunk.IDs->data()
	);
	glGenerateMipmap(GL_TEXTURE_3D);

	std::random_device rd;
	std::default_random_engine rd_engine(rd());
	std::uniform_int_distribution<int> dist(1,n_sprites);

	wlog.log(L"Loading maps.\n");

	MapLoader map;
	map.load("./assets/minecraft/region/r.0.0.mca", 0, 0);
	wlog.log(L"Loaded map (0,0).\n");
	// map.load("./assets/minecraft/region/r.1.0.mca", 1, 0);
	// wlog.log(L"Loaded map (1,0).\n");
	// map.load("./assets/minecraft/region/r.0.1.mca", 0, 1);
	// wlog.log(L"Loaded map (0,1).\n");
	// map.load("./assets/minecraft/region/r.1.1.mca", 1, 1);
	// wlog.log(L"Loaded map (1,1).\n");

	wlog.log(L"Creating Chunk Info Textures.\n");

	for(unsigned int x=0;x<chunks.size();++x) {
		for(unsigned int y=0;y<chunks[x].size();++y) {
			chunks[x][y].tex = 1;
			chunks[x][y].texnum = GL_TEXTURE0 + 1;
			chunks[x][y].position = glm::vec3(x, y, 0.f);

			if(map.regions[x/32][y/32].chunks[(y%32)*32+(x%32)].loaded) {
				chunks[x][y].IDs = &map.regions[x/32][y/32].chunks[(y%32)*32+(x%32)].blocks;
			}

			else {
				chunks[x][y].IDs = new std::vector<block_id>(chunk_total);
				for(int _z=0;_z<chunk_size.z;++_z) {
					for(int _y=0;_y<chunk_size.y;++_y) {
						for(int _x=0;_x<chunk_size.x;++_x) {
							int height = abs(_x-(chunk_size.x/2)) 
							           + abs(_y-(chunk_size.y/2));
							size_t index = _z*chunk_size.x*chunk_size.y
							             + _y*chunk_size.x
							             + _x;
							(*chunks[x][y].IDs)[index] =
								(_z>height)?dist(rd_engine):0;
						}
					}
				}
			}

			glActiveTexture(chunks[x][y].texnum);
			glGenTextures(1, &chunks[x][y].texid);
			glBindTexture(GL_TEXTURE_3D, chunks[x][y].texid);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);
			glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, col);
			glTexImage3D(
				GL_TEXTURE_3D, 0, GL_R8UI, chunk_size.x, chunk_size.y, 
				chunk_size.z, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 
				chunks[x][y].IDs->data()
			);
		}
	}
	process_gl_errors();

	glUseProgram(render_program);

	glViewport(0.f, 0.f, win_size_x, win_size_y);

	wlog.log(L"Creating and getting transform uniform data.\n");
	GLint model_uni = glGetUniformLocation(render_program, "model");

	process_gl_errors();


	wlog.log(L"Creating and getting camera position uniform data.\n");

	camera cam;

	process_gl_errors();

	wlog.log(L"Creating and getting view uniform data.\n");
	glm::mat4 view = cam.get_view();
	GLint view_uni = glGetUniformLocation(render_program, "view");
	glUniformMatrix4fv(view_uni, 1, GL_FALSE, glm::value_ptr(view));

	process_gl_errors();


	wlog.log(L"Creating and getting projection uniform data.\n");
	glm::mat4 projection = glm::perspective(
		pi/3.f, render_size.x/render_size.y, 0.01f, 3000.0f
	);
	GLint projection_uni = glGetUniformLocation(render_program, "projection");
	glUniformMatrix4fv(projection_uni, 1, GL_FALSE, glm::value_ptr(projection));

	GLint render_spritesheet_uni = glGetUniformLocation(render_program, "spritesheet");

	process_gl_errors();

	glUseProgram(lighting_program);

	GLint light_color_uni = glGetUniformLocation(lighting_program, "colorTex");
	GLint light_normals_uni = glGetUniformLocation(lighting_program, "normalsTex");
	GLint light_depth_uni = glGetUniformLocation(lighting_program, "depthTex");
	GLint light_proj_uni = glGetUniformLocation(lighting_program, "projection");
	glUniformMatrix4fv(light_proj_uni, 1, GL_FALSE, glm::value_ptr(projection));
	GLint light_view_uni = glGetUniformLocation(lighting_program, "view");
	glUniformMatrix4fv(light_view_uni, 1, GL_FALSE, glm::value_ptr(view));

	LightArray lights;
	lights.light_count = 1;
	lights.lights[0].brightness = 1.7f;
	lights.lights[0].radius = 500.f;
	lights.lights[0].fade = 30.f;
	lights.lights[0].position = glm::vec4(8.f, 8.f, 70.f, 1.f);
	lights.lights[0].color = glm::vec4(1.f, 0.9f, 1.f, 1.f);
	GLuint light_buffer;
	glGenBuffers(1, &light_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, light_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(lights), &lights, GL_STREAM_COPY);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, light_buffer);


	GLint light_intensity_uni = glGetUniformLocation(lighting_program, "intensity");
	GLint light_bias_uni = glGetUniformLocation(lighting_program, "bias");
	GLint light_rad_uni = glGetUniformLocation(lighting_program, "sample_radius");
	GLint light_scale_uni = glGetUniformLocation(lighting_program, "scale");

	glUseProgram(frustum_culling_program);

	GLint frustum_view_uni = glGetUniformLocation(frustum_culling_program, "view");
	GLint frustum_proj_uni = glGetUniformLocation(frustum_culling_program, "proj");
	glUniformMatrix4fv(frustum_proj_uni, 1, GL_FALSE, glm::value_ptr(projection));
	GLint frustum_chunk_size_uni = glGetUniformLocation(frustum_culling_program, "chunkSize");
	glUniform3fv(frustum_chunk_size_uni, 1, glm::value_ptr(
		glm::vec3(chunk_size.x,chunk_size.y,chunk_size.z))
	);

	glUseProgram(generate_program);

	wlog.log(L"Setting up transform feedback.\n");

	wlog.log(L"Creating and setting block chunk texture uniform data.\n");
	GLint chunk_id_uni = glGetUniformLocation(generate_program, "IDTex");
	glUniform1i(chunk_id_uni, 1);

	wlog.log(L"Creating and setting block chunk neighbors uniform data.\n");
	GLint neighbor_id_uni = glGetUniformLocation(generate_program, "neighbors");

	wlog.log(L"Creating and setting block chunk is bottom uniform data.\n");
	GLint chunk_is_bottom_id_uni = glGetUniformLocation(generate_program, "chunkIsBottom");

	wlog.log(L"Creating and setting block chunk size uniform data.\n");
	GLint chunk_size_uni = glGetUniformLocation(generate_program, "chunkSize");
	glUniform3fv(chunk_size_uni, 1, glm::value_ptr(
		glm::vec3(chunk_size.x,chunk_size.y,chunk_size.z))
	);

	process_gl_errors();

	GLuint generate_vao;
	glGenVertexArrays(1, &generate_vao);
	glBindVertexArray(generate_vao);

	wlog.log(L"Setting position vertex attribute data.\n");
	GLint gen_pos_attrib = glGetAttribLocation(generate_program, "pos");
	if(gen_pos_attrib != -1) {
		glEnableVertexAttribArray(gen_pos_attrib);
		glVertexAttribPointer(gen_pos_attrib, 3, GL_UNSIGNED_BYTE, GL_FALSE, 3, 0);
	}


	wlog.log(L"Generating chunk buffers.\n");
	auto start_tf = std::chrono::high_resolution_clock::now();


	std::vector<GLboolean> ssbo_null(chunk_total*24, GL_FALSE);
	GLuint ssbo;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, chunk_total*24*sizeof(GLboolean), ssbo_null.data(), GL_STREAM_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

	GLuint tbo;
	GLuint tfo;
	glGenTransformFeedbacks(1, &tfo);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfo);
	glGenBuffers(1, &tbo);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, tbo);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(float)*chunk_total*12*3*components_per_vtx, nullptr, GL_STREAM_COPY);

	uint64_t chunk_total_primitives = 0;
	uint64_t chunk_total_vertices = 0;
	uint64_t chunk_total_components = 0;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLuint query;
	glGenQueries(1, &query);
	for(unsigned int x=0;x<chunks.size();++x) {
		for(unsigned int y=0;y<chunks[x].size();++y) {
			glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, tbo);
			glEnable(GL_RASTERIZER_DISCARD);
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);

			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBindVertexArray(generate_vao);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_3D, empty_chunk.texid);
			empty_chunk.tex = 0;

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_3D, chunks[x][y].texid);
			chunks[x][y].tex = 1;


			// Get neighboring chunks, or set to empty_chunk if none
			GLint neighbor_tex[6];
			neighbor_tex[0]=0;
			glActiveTexture(GL_TEXTURE2);
			if(x) {
				glBindTexture(GL_TEXTURE_3D, chunks[x-1][y].texid);
				neighbor_tex[1]=2;
			}
			else {
				neighbor_tex[1]=0;
			}
			glActiveTexture(GL_TEXTURE3);
			if(y) {
				glBindTexture(GL_TEXTURE_3D, chunks[x][y-1].texid);
				neighbor_tex[2]=3;
			}
			else {
				neighbor_tex[2]=0;
			}
			glActiveTexture(GL_TEXTURE4);
			if(x<num_chunks.x-1) {
				glBindTexture(GL_TEXTURE_3D, chunks[x+1][y].texid);
				neighbor_tex[3]=4;
			}
			else {
				neighbor_tex[3]=0;
			}
			glActiveTexture(GL_TEXTURE5);
			if(y<num_chunks.y-1) {
				glBindTexture(GL_TEXTURE_3D, chunks[x][y+1].texid);
				neighbor_tex[4]=5;
			}
			else {
				neighbor_tex[4]=0;
			}
			neighbor_tex[5]=0;

			glUniform1i(chunk_id_uni, chunks[x][y].tex);
			//This is always true for now, as we only have a world height
			//  of 1 chunk
			glUniform1i(chunk_is_bottom_id_uni, true);
			glUniform1iv(neighbor_id_uni, 6, neighbor_tex);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
			glBufferData(GL_SHADER_STORAGE_BUFFER, chunk_total*24*sizeof(GLboolean), ssbo_null.data(), GL_STREAM_COPY);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo);

			glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
			glBeginTransformFeedback(GL_TRIANGLES);
				for(int i = 0; i < chunk_total; ++i) {
					glDrawArrays(GL_POINTS, i, 1);
					glFinish();
				}
			glEndTransformFeedback();
			glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

			glDisable(GL_RASTERIZER_DISCARD);

			GLuint primitives;
			glGetQueryObjectuiv(query, GL_QUERY_RESULT, &primitives);

			glGenBuffers(1, &chunks[x][y].buffer_geometry);
			glBindBuffer(GL_COPY_WRITE_BUFFER, chunks[x][y].buffer_geometry);
			glBufferData(GL_COPY_WRITE_BUFFER, sizeof(float)*primitives*3*components_per_vtx, nullptr, GL_STATIC_COPY);
			glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
			glBindBuffer(GL_COPY_READ_BUFFER , tbo);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(float)*primitives*3*components_per_vtx);
			glBindBuffer(GL_COPY_READ_BUFFER , 0);
			glBindBuffer(GL_COPY_WRITE_BUFFER , 0);
			chunks[x][y].primitive_count = primitives;
			chunks[x][y].vertex_count = primitives*3;
			chunks[x][y].component_count = chunks[x][y].vertex_count*components_per_vtx;
			chunk_total_primitives += chunks[x][y].primitive_count;
			chunk_total_vertices += chunks[x][y].vertex_count;
			chunk_total_components += chunks[x][y].component_count;
			wlog.log(
				L"Chunk["+std::to_wstring(x)+L"]["+std::to_wstring(y)+L"] buffer size: "
				+ std::to_wstring(chunks[x][y].component_count*sizeof(float))
				+ L"; total: "
				+ std::to_wstring(chunk_total_components*sizeof(float))
				+ L"\n"
			);
			glBindBuffer(GL_ARRAY_BUFFER, chunks[x][y].buffer_geometry);
			glGenVertexArrays(1, &chunks[x][y].vtx_array);
			glBindVertexArray(chunks[x][y].vtx_array);
			GLint pos_attrib = glGetAttribLocation(render_program, "pos");
			if(pos_attrib != -1) {
				glEnableVertexAttribArray(pos_attrib);
				glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, components_per_vtx*sizeof(GLfloat), BUFFER_OFFSET(sizeof(float)*0));
			}

			GLint texcoord_attrib = glGetAttribLocation(render_program, "texcoords");
			if(texcoord_attrib != -1) {
				glEnableVertexAttribArray(texcoord_attrib);
				glVertexAttribPointer(texcoord_attrib, 3, GL_FLOAT, GL_FALSE, components_per_vtx*sizeof(GLfloat), BUFFER_OFFSET(sizeof(float)*3));
			}

			GLint normal_attrib = glGetAttribLocation(render_program, "normal");
			if(normal_attrib != -1) {
				glEnableVertexAttribArray(normal_attrib);
				glVertexAttribPointer(normal_attrib, 3, GL_FLOAT, GL_FALSE, components_per_vtx*sizeof(GLfloat), BUFFER_OFFSET(sizeof(float)*6));
			}
		}
	}

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	glDeleteTransformFeedbacks(1, &tfo);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
	glDeleteBuffers(1, &tbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glDeleteBuffers(1, &ssbo);
	glDeleteVertexArrays(1, &generate_vao);
	glBindVertexArray(0);
	ssbo_null.clear();

	std::vector<std::vector<chunk_group>> chunk_groupings(
		num_chunks.x/chunk_grouping.x, std::vector<chunk_group>{num_chunks.y/chunk_grouping.y}
	);

	for(int x = 0; x < num_chunks.x/chunk_grouping.x; ++x) {
		for(int y = 0; y < num_chunks.y/chunk_grouping.y; ++y) {
			size_t sum = 0;
			for(int i = 0; i < chunk_grouping.x; ++i) {
				for(int j = 0; j < chunk_grouping.y; ++j) {
					chunk_groupings[x][y].chunks.push_back(&chunks[x*chunk_grouping.x+i][y*chunk_grouping.y+j]);
					sum += chunks[x*chunk_grouping.x+i][y*chunk_grouping.y+j].component_count * sizeof(float);
				}
			}
			glGenBuffers(1, &chunk_groupings[x][y].merged_geometry);
			glBindBuffer(GL_COPY_WRITE_BUFFER, chunk_groupings[x][y].merged_geometry);
			glBufferData(GL_COPY_WRITE_BUFFER, sum, nullptr, GL_STATIC_COPY);
			sum = 0;
			for(auto &c : chunk_groupings[x][y].chunks) {
				glBindBuffer(GL_COPY_READ_BUFFER , c->buffer_geometry);
				glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, sum, c->component_count*sizeof(float));
				glBindBuffer(GL_COPY_READ_BUFFER, 0);
				glDeleteBuffers(1, &c->buffer_geometry);
				sum += c->component_count*sizeof(float);
			}

			glBindBuffer(GL_COPY_READ_BUFFER, 0);
			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

			glBindBuffer(GL_ARRAY_BUFFER, chunk_groupings[x][y].merged_geometry);
			glGenVertexArrays(1, &chunk_groupings[x][y].vtx_array);
			glBindVertexArray(chunk_groupings[x][y].vtx_array);
			GLint pos_attrib = glGetAttribLocation(render_program, "pos");
			if(pos_attrib != -1) {
				glEnableVertexAttribArray(pos_attrib);
				glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, components_per_vtx*sizeof(GLfloat), BUFFER_OFFSET(sizeof(float)*0));
			}

			GLint texcoord_attrib = glGetAttribLocation(render_program, "texcoords");
			if(texcoord_attrib != -1) {
				glEnableVertexAttribArray(texcoord_attrib);
				glVertexAttribPointer(texcoord_attrib, 3, GL_FLOAT, GL_FALSE, components_per_vtx*sizeof(GLfloat), BUFFER_OFFSET(sizeof(float)*3));
			}

			GLint normal_attrib = glGetAttribLocation(render_program, "normal");
			if(normal_attrib != -1) {
				glEnableVertexAttribArray(normal_attrib);
				glVertexAttribPointer(normal_attrib, 3, GL_FLOAT, GL_FALSE, components_per_vtx*sizeof(GLfloat), BUFFER_OFFSET(sizeof(float)*6));
			}
		}
	}

	auto end_tf = std::chrono::high_resolution_clock::now();

	auto time_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end_tf-start_tf);

	wlog.log(L"Done generating chunk buffers.\n");
	wlog.log(L"Generated "+ std::to_wstring(chunks.size()*chunks[0].size())+L" chunks in "+std::to_wstring(time_elapsed.count())+L"µs.\n");
	wlog.log(
		L"Chunk buffers total: {primitives: "
		+ std::to_wstring(chunk_total_primitives)
		+ L", vertices: "
		+ std::to_wstring(chunk_total_vertices)
		+ L", components: "
		+ std::to_wstring(chunk_total_components)
		+ L", bytes: "
		+ std::to_wstring(chunk_total_components*sizeof(float))
		+ L"}.\n"
	);

	for(auto cx : chunks) {
		for(auto cy : cx) {
			glDeleteTextures(1, &cy.texid);
		}
	}

	wlog.log(L"Starting main loop.\n");

	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	GLuint framebuffer_render;
	glGenFramebuffers(1, &framebuffer_render);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_render);

	GLuint framebuffer_render_color_texture;
	glGenTextures(1, &framebuffer_render_color_texture);
	glActiveTexture(GL_TEXTURE0+4);
	glProgramUniform1i(lighting_program, light_color_uni, 4);
	glBindTexture(GL_TEXTURE_2D, framebuffer_render_color_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, render_size.x, render_size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer_render_color_texture, 0);

	GLuint framebuffer_render_normals_texture;
	glGenTextures(1, &framebuffer_render_normals_texture);
	glActiveTexture(GL_TEXTURE0+5);
	glProgramUniform1i(lighting_program, light_normals_uni, 5);
	glBindTexture(GL_TEXTURE_2D, framebuffer_render_normals_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, render_size.x, render_size.y, 0, GL_RG, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, framebuffer_render_normals_texture, 0);
	
	GLuint framebuffer_render_depth_texture;
	glGenTextures(1, &framebuffer_render_depth_texture);
	glActiveTexture(GL_TEXTURE0+6);
	glProgramUniform1i(lighting_program, light_depth_uni, 6);
	glBindTexture(GL_TEXTURE_2D, framebuffer_render_depth_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, render_size.x, render_size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, framebuffer_render_depth_texture, 0);

	GLenum drawbuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_DEPTH_ATTACHMENT};
	glDrawBuffers(2, drawbuffers);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		wlog.log("Incomplete framebuffer!\n");

	GLuint framebuffer_display;
	glGenFramebuffers(1, &framebuffer_display);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_display);

	GLint framebuffer_uni = glGetUniformLocation(display_program, "framebuffer");

	glGenTextures(1, &framebuffer_display_color_texture);
	glActiveTexture(GL_TEXTURE0+7);
	glProgramUniform1i(display_program, framebuffer_uni, 7);
	glBindTexture(GL_TEXTURE_2D, framebuffer_display_color_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, render_size.x, render_size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer_display_color_texture, 0);

	float fb_vertices[] = {
		// Coords  Texcoords
		-1.f,  1.f,   0.f, 1.f,
		 1.f, -1.f,   1.f, 0.f,
		-1.f, -1.f,   0.f, 0.f,
		 1.f,  1.f,   1.f, 1.f,
		 1.f, -1.f,   1.f, 0.f,
		-1.f,  1.f,   0.f, 1.f,
	};
	GLuint fb_vbo, fb_vao;
	glGenVertexArrays(1, &fb_vao);
	glBindVertexArray(fb_vao);
	glGenBuffers(1, &fb_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, fb_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fb_vertices), fb_vertices, GL_STATIC_DRAW);

	GLint fb_vao_pos_attrib = glGetAttribLocation(display_program, "pos");
	if(fb_vao_pos_attrib != -1) {
		glEnableVertexAttribArray(fb_vao_pos_attrib);
		glVertexAttribPointer(fb_vao_pos_attrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
	}

	GLint fb_vao_texcoord_attrib = glGetAttribLocation(display_program, "texcoords");
	if(fb_vao_texcoord_attrib != -1) {
		glEnableVertexAttribArray(fb_vao_texcoord_attrib);
		glVertexAttribPointer(fb_vao_texcoord_attrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), BUFFER_OFFSET(sizeof(float)*2));
	}

	GLuint fc_tbo;
	GLuint fc_tfo;
	glGenTransformFeedbacks(1, &fc_tfo);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, fc_tfo);
	glGenBuffers(1, &fc_tbo);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, fc_tbo);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(float)*num_chunks.x*num_chunks.y*3, nullptr, GL_STREAM_COPY);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLuint fc_vbo;
	glGenBuffers(1, &fc_vbo);
	GLuint fc_vao;
	glGenVertexArrays(1, &fc_vao);
	GLuint fc_query;
	glGenQueries(1, &fc_query);
	GLuint fc_primitives;

	glBindBuffer(GL_ARRAY_BUFFER, fc_vbo);
	glBindVertexArray(fc_vao);

	std::vector<glm::ivec3> indices(num_chunks.x*num_chunks.y);
	for(int y=0;y<num_chunks.y;++y) {
		for(int x=0;x<num_chunks.x;++x) {
			indices[x+y*num_chunks.x] = glm::ivec3(x, y, 0);
		}
	}

	std::vector<glm::ivec3> visible_indices(num_chunks.x*num_chunks.y);

	glBufferData(GL_ARRAY_BUFFER, sizeof(int)*3*num_chunks.x*num_chunks.y, indices.data(), GL_STATIC_DRAW);

	GLint fc_pos_attrib = glGetAttribLocation(frustum_culling_program, "pos");
	if(fc_pos_attrib != -1) {
		glEnableVertexAttribArray(fc_pos_attrib);
		glVertexAttribPointer(fc_pos_attrib, 3, GL_INT, GL_FALSE, 3*sizeof(int), BUFFER_OFFSET(sizeof(float)*0));
	}

	glUseProgram(render_program);

	glfwSetKeyCallback(win, [](GLFWwindow*, int key, int, int action, int){
		switch(action) {
			case GLFW_PRESS: {
				switch(key) {

				}
			} break;
			case GLFW_RELEASE: {
				switch(key) {
					case GLFW_KEY_F: {
						static bool wireframe=false;
						wireframe = !wireframe;
						if(wireframe)
							glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
						else
							glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					} break;
					case GLFW_KEY_U: {
						uint8_t *pixels = new uint8_t[static_cast<int>(render_size.x)*static_cast<int>(render_size.y)*4];
						glBindTexture(GL_TEXTURE_2D, framebuffer_display_color_texture);
						glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
						uint8_t *topbottom_pixels = new uint8_t[static_cast<int>(render_size.x)*static_cast<int>(render_size.y)*4];
						for(int y = 0; y < render_size.y; ++y) {
							for(int x = 0; x < render_size.x; ++x) {
								int ny = (render_size.y-1) - y;
								topbottom_pixels[(x+y*int(render_size.x))*4+0] = pixels[(x+ny*int(render_size.x))*4+0];
								topbottom_pixels[(x+y*int(render_size.x))*4+1] = pixels[(x+ny*int(render_size.x))*4+1];
								topbottom_pixels[(x+y*int(render_size.x))*4+2] = pixels[(x+ny*int(render_size.x))*4+2];
								topbottom_pixels[(x+y*int(render_size.x))*4+3] = pixels[(x+ny*int(render_size.x))*4+3];
							}
						}
						if(!stbi_write_png("/tmp/screenshot.png", render_size.x, render_size.y, 4, topbottom_pixels, 0)) {
							wlog.log(L"ERROR SAVING SCREENSHOT!\n");
						}
						else {
							wlog.log(L"????\n");
						}
						delete[] pixels;
					} break;
				}
			} break;
		}
	});

	std::chrono::high_resolution_clock::time_point start, end, timetoprint;
	timetoprint = end = start = std::chrono::high_resolution_clock::now();

	long long cnt=0;
	long double ft_total=0.f;

	glClearColor(0.517f, 0.733f, 0.996f, 1.f);

	float intensity = 0.91;
	float bias = 0.21;
	float scale = 0.27;
	float sample_radius = 0.20;

	while(!glfwWindowShouldClose(win)) {
		end = std::chrono::high_resolution_clock::now();
		long int ft = std::chrono::duration_cast<std::chrono::microseconds>(
			end-start
		).count();
		double fts = static_cast<double>(ft)/1e6L;
		float fts_float = static_cast<float>(fts);
		ft_total += ft;
		++cnt;
		auto tslastprint = std::chrono::duration_cast<std::chrono::seconds>(
			end-timetoprint
		).count();
		if(tslastprint >= 1) {
			timetoprint = std::chrono::high_resolution_clock::now();
			float ft_avg = ft_total/cnt;
			std::wstring frametimestr = L"FPS avg: " + 
				std::to_wstring(1e6L/ft_avg) + L"\t" +
				L"Frametime avg: "+std::to_wstring(ft_avg)+L"µs\n";
			wlog.log(frametimestr);
			cnt=0;
			ft_total=0.L;
			wlog.log(L"SSAO Intensity : \t" + std::to_wstring(intensity) + L"\n");
			wlog.log(L"SSAO Bias : \t" + std::to_wstring(bias) + L"\n");
			wlog.log(L"SSAO Scale : \t" + std::to_wstring(scale) + L"\n");
			wlog.log(L"SSAO Sample Radius : \t" + std::to_wstring(sample_radius) + L"\n");
		}
		start=end;

		if(glfwGetKey(win, GLFW_KEY_W)) {
			cam.rotate({-1.f, 0.f, 0.f}, fts_float*3.f);
		}
		if(glfwGetKey(win, GLFW_KEY_S)) {
			cam.rotate({ 1.f, 0.f, 0.f}, fts_float*3.f);
		}
		if(glfwGetKey(win, GLFW_KEY_A)) {
			cam.rotate({ 0.f,-1.f, 0.f}, fts_float*3.f);
		}
		if(glfwGetKey(win, GLFW_KEY_D)) {
			cam.rotate({ 0.f, 1.f, 0.f}, fts_float*3.f);
		}
		if(glfwGetKey(win, GLFW_KEY_Q)) {
			cam.rotate({ 0.f, 0.f,-1.f}, fts_float*3.f);
		}
		if(glfwGetKey(win, GLFW_KEY_E)) {
			cam.rotate({ 0.f, 0.f, 1.f}, fts_float*3.f);
		}
		if(glfwGetKey(win, GLFW_KEY_UP)) {
			cam.advance(fts_float*100.f);
		}
		if(glfwGetKey(win, GLFW_KEY_DOWN)) {
			cam.advance(fts_float*-100.f);
		}
		if(glfwGetKey(win, GLFW_KEY_RIGHT)) {
			cam.strafe(fts_float*100.f);
		}
		if(glfwGetKey(win, GLFW_KEY_LEFT)) {
			cam.strafe(fts_float*-100.f);
		}
		if(glfwGetKey(win, GLFW_KEY_SPACE)) {
			cam.climb(fts_float*100.f);
		}
		if(glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL)) {
			cam.climb(fts_float*-100.f);
		}

		if(glfwGetKey(win, GLFW_KEY_G)) {
			intensity += 0.01;
			glProgramUniform1f(lighting_program, light_intensity_uni, intensity);
		}
		if(glfwGetKey(win, GLFW_KEY_V)) {
			intensity -= 0.01;
			glProgramUniform1f(lighting_program, light_intensity_uni, intensity);
		}
		if(glfwGetKey(win, GLFW_KEY_H)) {
			bias += 0.01;
			glProgramUniform1f(lighting_program, light_bias_uni, bias);
		}
		if(glfwGetKey(win, GLFW_KEY_B)) {
			bias -= 0.01;
			glProgramUniform1f(lighting_program, light_bias_uni, bias);
		}
		if(glfwGetKey(win, GLFW_KEY_J)) {
			sample_radius += 0.01;
			glProgramUniform1f(lighting_program, light_rad_uni, sample_radius);
		}
		if(glfwGetKey(win, GLFW_KEY_N)) {
			sample_radius -= 0.01;
			glProgramUniform1f(lighting_program, light_rad_uni, sample_radius);
		}
		if(glfwGetKey(win, GLFW_KEY_K)) {
			scale += 0.01;
			glProgramUniform1f(lighting_program, light_scale_uni, scale);
		}
		if(glfwGetKey(win, GLFW_KEY_M)) {
			scale -= 0.01;
			glProgramUniform1f(lighting_program, light_scale_uni, scale);
		}

		glUseProgram(frustum_culling_program);

		glUniformMatrix4fv(frustum_view_uni, 1, GL_FALSE, glm::value_ptr(view));

		glBindBuffer(GL_ARRAY_BUFFER, fc_vbo);
		glBindVertexArray(fc_vao);
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, fc_tfo);
		glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, fc_tbo);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, fc_tbo);

		glEnable(GL_RASTERIZER_DISCARD);


		glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, fc_query);
		glBeginTransformFeedback(GL_POINTS);
			glDrawArrays(GL_POINTS, 0, num_chunks.x*num_chunks.y);
		glEndTransformFeedback();
		glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

		glDisable(GL_RASTERIZER_DISCARD);

		glGetQueryObjectuiv(fc_query, GL_QUERY_RESULT, &fc_primitives);

		glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, fc_primitives*sizeof(int)*3, visible_indices.data());

		glUseProgram(render_program);

		view = cam.get_view();
		glUniformMatrix4fv(view_uni, 1, GL_FALSE, glm::value_ptr(view));
		glProgramUniformMatrix4fv(lighting_program, light_view_uni, 1, GL_FALSE, glm::value_ptr(view));
		glUniform1i(render_spritesheet_uni, 0);


		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_render);
		glViewport(0.f, 0.f, render_size.x, render_size.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for(auto &cv : chunks) {
			for(auto &c : cv) {
				c.draw = false;
			}
		}

		for(unsigned int i = 0; i < fc_primitives; ++i) {
			unsigned int x = visible_indices[i].x;
		 	unsigned int y = visible_indices[i].y;
		 	chunks[x][y].draw = true;
		}

		for(auto &gv : chunk_groupings) {
			for(auto &g : gv) {
				bool bound = false;
				size_t sum = 0;
				for(auto &c : g.chunks) {
					if(c->draw) {
						if(!bound) {
							glBindBuffer(GL_ARRAY_BUFFER, g.merged_geometry);
							glBindVertexArray(g.vtx_array);
							bound = true;
						}
						glm::mat4 transform;
						transform = glm::translate(
							transform,
							glm::vec3(chunk_size.x, chunk_size.y, chunk_size.z)*
							glm::vec3(c->position)
						);
						glUniformMatrix4fv(
							model_uni, 1, GL_FALSE, glm::value_ptr(transform)
						);
						glDrawArrays(GL_TRIANGLES, sum, c->vertex_count);
					}
					sum += c->vertex_count;
				}
			}
		}

		glDisable(GL_DEPTH_TEST);
		
		GLint poly_mode;

		glGetIntegerv(GL_POLYGON_MODE, &poly_mode);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glBindVertexArray(fb_vao);
		glBindBuffer(GL_ARRAY_BUFFER, fb_vbo);

		glUseProgram(lighting_program);

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_display);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(display_program);
		glfwGetWindowSize(win, &win_size_x, &win_size_y);
		glViewport(0.f, 0.f, win_size_x, win_size_y);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glPolygonMode(GL_FRONT_AND_BACK, poly_mode);

		glEnable(GL_DEPTH_TEST);

		glfwSwapBuffers(win);
		glfwPollEvents();
		process_gl_errors();
	}

	glDeleteTextures(1, &empty_chunk.texid);
	delete empty_chunk.IDs;
	for(unsigned int x = 0; x < chunks.size(); ++x) {
		for(unsigned int y = 0; y < chunks[x].size(); ++y) {
			glDeleteBuffers(1, &(chunks[x][y].buffer_geometry));
			glDeleteVertexArrays(1, &(chunks[x][y].vtx_array));
		}
	}

	// glDeleteFramebuffers(1, &framebuffer);
	glDeleteShader(shader_render_vert);
	glDeleteShader(shader_render_frag);
	glDeleteShader(shader_generate_geom);
	glDeleteShader(shader_generate_vert);
	glDeleteProgram(render_program);
	glDeleteProgram(generate_program);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	glfwDestroyWindow(win);

	return cleanup(0);
}

int cleanup(int rtval, std::wstring extra)
{
	if(rtval == -1) {
		wlog.log(L"Could not initialize GLFW.\n");
		return rtval;
	}
	switch(rtval) {
		case  0: {
			wlog.log(L"Exiting..\n");
			break;
		}
		case -2: {
			wlog.log(L"Could not create window.\n");
			break;
		}
		case -3: {
			wlog.log(L"Could not initialize GLEW.\n");
			break;
		}
		case -4: {
			wlog.log(L"Could not read file: ");
			wlog.log(extra);
			wlog.log(L"\n");
			break;
		}
		case -5: {
			wlog.log(L"Shader compilation failed: ");
			wlog.log(extra);
			wlog.log("\n");
		}
	}
	glfwTerminate();
	return rtval;
}

bool process_gl_errors()
{
	GLenum gl_err;
	bool no_err=true;
	while((gl_err = glGetError()) != GL_NO_ERROR) {
		no_err=false;
		wlog.log("OpenGL Error:\n");
		switch(gl_err) {
			case GL_INVALID_ENUM:
				wlog.log("\tInvalid enum.\n");
				break;
			case GL_INVALID_VALUE:
				wlog.log("\tInvalid value.\n");
				break;
			case GL_INVALID_OPERATION:
				wlog.log("\tInvalid operation.\n");
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				wlog.log("\tInvalid framebuffer operation.\n");
				break;
			case GL_OUT_OF_MEMORY:
				wlog.log("\tOut of memory.\n");
				break;
		}
	}
	return no_err;
}
