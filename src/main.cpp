#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Logger/Logger.hpp"
#include "Shader/Shader.hpp"
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


// Common macro for casting OpenGL buffer offsets
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

constexpr float init_win_size_x = 960.f;
constexpr float init_win_size_y = 540.f;

constexpr float render_size_x = 3840.f;
constexpr float render_size_y = 2160.f;

using coord_type = uint8_t;
using block_id = uint8_t;

constexpr GLsizei components_per_vtx = 9;

//Specify amount of chunks
constexpr int32_t   chunks_x=32;
constexpr int32_t   chunks_y=32;
constexpr int32_t   chunks_z=0; // Unused for now
// Specify chunk sizes, chunk_size_*  and chunk_total must be a power of 2.
constexpr int32_t chunk_size_x=16;
constexpr int32_t chunk_size_y=16;
constexpr int32_t chunk_size_z=256;
constexpr uint64_t   chunk_total =chunk_size_x*chunk_size_y*chunk_size_z;

// Camera struct
struct camera {
	// Static members for reference data
	static const glm::vec3 _direction;
	static const glm::vec3 _up;
	static const glm::vec3 _right;
	glm::vec3 up = _up;
	glm::vec3 direction = _direction;
	glm::vec3 right = _right;
	glm::vec3 position;
	glm::quat orientation;
};

const glm::vec3 camera::_direction = glm::vec3( 1.f, 0.f, 0.f);
const glm::vec3 camera::_up        = glm::vec3( 0.f, 0.f, -1.f);
const glm::vec3 camera::_right     = glm::vec3( 0.f, 1.f, 0.f);

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
	static const glm::ivec3 chunk_size;
	static std::array<block, chunk_total> *offsets;
	glm::ivec3 position;
	std::array<block_id, chunk_total> *IDs;
	GLenum texnum;
	GLuint texid;
	GLuint tex;
	GLuint buffer_geometry;
	GLuint primitive_count;
	GLuint vertex_count;
	GLuint component_count;
	GLuint vtx_array;
};

const glm::ivec3 chunk::chunk_size = glm::ivec3(
	chunk_size_x, chunk_size_y, chunk_size_z
);

std::array<block, chunk_total> *chunk::offsets = 
	new std::array<block, chunk_total>;

constexpr int block_offset_x = 0;
constexpr int block_offset_y = block_offset_x + sizeof(coord_type);
constexpr int block_offset_z = block_offset_y + sizeof(coord_type);

GLuint framebuffer_display_color_texture;

constexpr float pi = 3.14159;

Logger<wchar_t> wlog{std::wcout};

int cleanup(int rtval, std::wstring extra=L"");
bool readfile(const char* filename, std::string &contents);
bool process_gl_errors();

int main()
{
	// Generate chunk_x*chunk_y chunks
	std::vector<std::vector<chunk>> chunks(chunks_x);
	for(auto &v : chunks) {
		v.resize(chunks_y);
		for(auto &c : v) {
			c = chunk();
		}
	}

	// Fill chunk offsets with.. their offsets
	for(int z=0;z<chunk_size_z;++z) {
		for(int y=0;y<chunk_size_y;++y) {
			for(int x=0;x<chunk_size_x;++x) {
				(*chunk::offsets)[z*chunk_size_x*chunk_size_y+y*chunk_size_x+x] = block{x,y,z};
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow *win = glfwCreateWindow(
		init_win_size_x, init_win_size_y, "Voxelator!", nullptr, nullptr
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

	process_gl_errors();

	wlog.log("Generating Vertex Array Object.\n");
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	process_gl_errors();

	wlog.log(L"Creating Shaders.\n");

	Shader shader_generate_vert;
	shader_generate_vert.load_file(GL_VERTEX_SHADER, "assets/shaders/generate/shader.vert");
	process_gl_errors();

	wlog.log(L"Creating generate geometry shader.\n");
	Shader shader_generate_geom;
	shader_generate_geom.load_file(GL_GEOMETRY_SHADER, "assets/shaders/generate/shader.geom");
	process_gl_errors();

	wlog.log(L"Creating and linking generate shader program.\n");
	GLuint generate_program = glCreateProgram();
	glAttachShader(generate_program, shader_generate_vert);
	glAttachShader(generate_program, shader_generate_geom);
	const char *varyings[] = {"gPos", "gTexcoords", "gNormal"};
	glTransformFeedbackVaryings(generate_program, 3, varyings, GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(generate_program);
	glUseProgram(generate_program);

	process_gl_errors();

	wlog.log(L"Creating frustum_culling vertex shader.\n");
	Shader shader_frustum_culling_vert;
	shader_frustum_culling_vert.load_file(GL_VERTEX_SHADER, "assets/shaders/frustum_culling/shader.vert");
	process_gl_errors();

	wlog.log(L"Creating frustum_culling geometry shader.\n");
	Shader shader_frustum_culling_geom;
	shader_frustum_culling_geom.load_file(GL_GEOMETRY_SHADER, "assets/shaders/frustum_culling/shader.geom");
	process_gl_errors();

	wlog.log(L"Creating and linking frustum_culling shader program.\n");
	GLuint frustum_culling_program = glCreateProgram();
	glAttachShader(frustum_culling_program, shader_frustum_culling_vert);
	glAttachShader(frustum_culling_program, shader_frustum_culling_geom);
	const char *fcvaryings[] = {"gPos"};
	glTransformFeedbackVaryings(frustum_culling_program, 1, fcvaryings, GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(frustum_culling_program);
	glUseProgram(frustum_culling_program);

	process_gl_errors();
	
	wlog.log(L"Creating render vertex shader.\n");
	Shader shader_render_vert;
	shader_render_vert.load_file(GL_VERTEX_SHADER, "assets/shaders/render/shader.vert");
	process_gl_errors();

	wlog.log(L"Creating render fragment shader.\n");
	Shader shader_render_frag;
	shader_render_frag.load_file(GL_FRAGMENT_SHADER, "assets/shaders/render/shader.frag");
	process_gl_errors();

	wlog.log(L"Creating and linking render shader program.\n");
	GLuint render_program = glCreateProgram();
	glAttachShader(render_program, shader_render_vert);
	glAttachShader(render_program, shader_render_frag);
	glBindFragDataLocation(render_program, 0, "outColor");
	glBindFragDataLocation(render_program, 1, "outNormal");
	glLinkProgram(render_program);
	glUseProgram(render_program);

	process_gl_errors();

	wlog.log(L"Creating lighting vertex shader.\n");
	Shader shader_lighting_vert;
	shader_lighting_vert.load_file(GL_VERTEX_SHADER, "assets/shaders/lighting/shader.vert");
	process_gl_errors();

	wlog.log(L"Creating lighting fragment shader.\n");
	Shader shader_lighting_frag;
	shader_lighting_frag.load_file(GL_FRAGMENT_SHADER, "assets/shaders/lighting/shader.frag");

	process_gl_errors();

	wlog.log(L"Creating and linking lighting shader program.\n");
	GLuint lighting_program = glCreateProgram();
	glAttachShader(lighting_program, shader_lighting_vert);
	glAttachShader(lighting_program, shader_lighting_frag);
	glBindFragDataLocation(lighting_program, 0, "outCol");
	glLinkProgram(lighting_program);
	glUseProgram(lighting_program);

	process_gl_errors();

	wlog.log(L"Creating display vertex shader.\n");
	Shader shader_display_vert;
	shader_display_vert.load_file(GL_VERTEX_SHADER, "assets/shaders/display/shader.vert");
	process_gl_errors();

	wlog.log(L"Creating display fragment shader.\n");
	Shader shader_display_frag;
	shader_display_frag.load_file(GL_FRAGMENT_SHADER, "assets/shaders/display/shader.frag");
	process_gl_errors();

	wlog.log(L"Creating and linking display shader program.\n");
	GLuint display_program = glCreateProgram();
	glAttachShader(display_program, shader_display_vert);
	glAttachShader(display_program, shader_display_frag);
	glBindFragDataLocation(display_program, 0, "color");
	glLinkProgram(display_program);
	glUseProgram(display_program);

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
		"assets/images/spritesheet2a.png", &spritesheet_x, &spritesheet_y,
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
	glTexImage3D(
		GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, sprite_size.x, sprite_size.y, sprites, 0, 
		GL_RGBA, GL_UNSIGNED_BYTE, spritesheet_data
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
		GL_ARRAY_BUFFER, sizeof(*chunk::offsets),
		chunk::offsets->data(), GL_STATIC_DRAW
	);
	process_gl_errors();

	wlog.log(L"Creating Chunk Info Textures.\n");

	chunk empty_chunk;
	empty_chunk.buffer_geometry=-1;
	empty_chunk.component_count=-1;
	empty_chunk.primitive_count=-1;
	empty_chunk.vertex_count=-1;
	empty_chunk.vtx_array=-1;
	empty_chunk.IDs = new std::array<block_id, chunk_total>;
	empty_chunk.IDs->fill(0);
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
		GL_TEXTURE_3D, 0, GL_RED, chunk_size_x, chunk_size_y, 
		chunk_size_z, 0, GL_RED, GL_UNSIGNED_BYTE, 
		empty_chunk.IDs->data()
	);
	glGenerateMipmap(GL_TEXTURE_3D);

	std::random_device rd;
	std::default_random_engine rd_engine(rd());
	std::uniform_int_distribution<int> dist(1,n_sprites);

	for(unsigned int x=0;x<chunks.size();++x) {
		for(unsigned int y=0;y<chunks[x].size();++y) {
			int i = x*chunks[x].size()+y;
			chunks[x][y].tex = 1;
			chunks[x][y].texnum = GL_TEXTURE0 + 1;
			chunks[x][y].IDs = new std::array<block_id, chunk_total>;
			chunks[x][y].position = glm::vec3(x, y, 0.f);

			for(int _z=0;_z<chunk_size_z;++_z) {
				for(int _y=0;_y<chunk_size_y;++_y) {
					for(int _x=0;_x<chunk_size_x;++_x) {
						int height = abs(_x-(chunk_size_x/2)) 
						           + abs(_y-(chunk_size_y/2));
						size_t index = _z*chunk_size_x*chunk_size_y
						             + _y*chunk_size_x
						             + _x;
						(*chunks[x][y].IDs)[index] =
							(_z>height)?dist(rd_engine):0;	
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
				GL_TEXTURE_3D, 0, GL_R8UI, chunk_size_x, chunk_size_y, 
				chunk_size_z, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 
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
	cam.up = camera::_up;
	cam.direction = camera::_direction;
	cam.right = camera::_right;
	cam.orientation = glm::quat();

	process_gl_errors();

	wlog.log(L"Creating and getting view uniform data.\n");
	glm::mat4 view = glm::lookAt(
		cam.position,
		cam.direction,
		cam.up
	);
	GLint view_uni = glGetUniformLocation(render_program, "view");
	glUniformMatrix4fv(view_uni, 1, GL_FALSE, glm::value_ptr(view));

	process_gl_errors();


	wlog.log(L"Creating and getting projection uniform data.\n");
	glm::mat4 projection = glm::perspective(
		pi/3.f, render_size_x/render_size_y, 0.01f, 3000.0f
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

	glUseProgram(frustum_culling_program);

	GLint frustum_view_uni = glGetUniformLocation(frustum_culling_program, "view");
	GLint frustum_proj_uni = glGetUniformLocation(frustum_culling_program, "proj");
	glUniformMatrix4fv(frustum_proj_uni, 1, GL_FALSE, glm::value_ptr(projection));
	GLint frustum_chunk_size_uni = glGetUniformLocation(frustum_culling_program, "chunkSize");
	glUniform3fv(frustum_chunk_size_uni, 1, glm::value_ptr(
		glm::vec3(chunk_size_x,chunk_size_y,chunk_size_z))
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
		glm::vec3(chunk_size_x,chunk_size_y,chunk_size_z))
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
			glActiveTexture(GL_TEXTURE2);
			if(x<chunks_x-1) {
				glBindTexture(GL_TEXTURE_3D, chunks[x+1][y].texid);
				neighbor_tex[0]=2;
			}
			else {
				neighbor_tex[0]=0;
			}
			glActiveTexture(GL_TEXTURE3);
			if(y<chunks_y-1) {
				glBindTexture(GL_TEXTURE_3D, chunks[x][y+1].texid);
				neighbor_tex[1]=3;
			}
			else {
				neighbor_tex[1]=0;
			}
			neighbor_tex[2]=0;
			glActiveTexture(GL_TEXTURE4);
			if(x) {
				glBindTexture(GL_TEXTURE_3D, chunks[x-1][y].texid);
				neighbor_tex[3]=4;
			}
			else {
				neighbor_tex[3]=0;
			}
			glActiveTexture(GL_TEXTURE5);
			if(y) {
				glBindTexture(GL_TEXTURE_3D, chunks[x][y-1].texid);
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

			glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
			glBeginTransformFeedback(GL_TRIANGLES);
				glDrawArrays(GL_POINTS, 0, chunk_total);
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
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &generate_vao);

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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, render_size_x, render_size_y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer_render_color_texture, 0);

	GLuint framebuffer_render_normals_texture;
	glGenTextures(1, &framebuffer_render_normals_texture);
	glActiveTexture(GL_TEXTURE0+5);
	glProgramUniform1i(lighting_program, light_normals_uni, 5);
	glBindTexture(GL_TEXTURE_2D, framebuffer_render_normals_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, render_size_x, render_size_y, 0, GL_RG, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, framebuffer_render_normals_texture, 0);
	
	GLuint framebuffer_render_depth_texture;
	glGenTextures(1, &framebuffer_render_depth_texture);
	glActiveTexture(GL_TEXTURE0+6);
	glProgramUniform1i(lighting_program, light_depth_uni, 6);
	glBindTexture(GL_TEXTURE_2D, framebuffer_render_depth_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, render_size_x, render_size_y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, render_size_x, render_size_y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
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
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(float)*chunks_x*chunks_y*3, nullptr, GL_STREAM_COPY);

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

	glm::ivec3 *indices = new glm::ivec3[chunks_x*chunks_y];
	for(int y=0;y<chunks_y;++y) {
		for(int x=0;x<chunks_x;++x) {
			indices[x+y*chunks_x] = glm::ivec3(x, y, 0);
		}
	}

	glm::ivec3 *visible_indices = new glm::ivec3[chunks_x*chunks_y];

	glBufferData(GL_ARRAY_BUFFER, sizeof(int)*3*chunks_x*chunks_y, indices, GL_STATIC_DRAW);

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
						uint8_t *pixels = new uint8_t[static_cast<int>(render_size_x)*static_cast<int>(render_size_y)*4];
						glBindTexture(GL_TEXTURE_2D, framebuffer_display_color_texture);
						glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
						uint8_t *topbottom_pixels = new uint8_t[static_cast<int>(render_size_x)*static_cast<int>(render_size_y)*4];
						for(int y = 0; y < render_size_y; ++y) {
							for(int x = 0; x < render_size_x; ++x) {
								int ny = (render_size_y-1) - y;
								topbottom_pixels[(x+y*int(render_size_x))*4+0] = pixels[(x+ny*int(render_size_x))*4+0];
								topbottom_pixels[(x+y*int(render_size_x))*4+1] = pixels[(x+ny*int(render_size_x))*4+1];
								topbottom_pixels[(x+y*int(render_size_x))*4+2] = pixels[(x+ny*int(render_size_x))*4+2];
								topbottom_pixels[(x+y*int(render_size_x))*4+3] = pixels[(x+ny*int(render_size_x))*4+3];
							}
						}
						if(!stbi_write_png("/tmp/screenshot.png", render_size_x, render_size_y, 4, topbottom_pixels, 0)) {
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

	glFlush();

	long long cnt=0;
	long double ft_total=0.f;

	cam.position = glm::vec3(chunks_x*chunk_size_x/2.f, chunks_y*chunk_size_y/2.f, 0.f);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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
			wlog.log(L"Direction:\t");
			wlog.log(cam.direction, false);
			wlog.log(L"\n", false);
			wlog.log(L"Up:\t\t");
			wlog.log(cam.up, false);
			wlog.log(L"\n", false);
			wlog.log(L"Right:\t");
			wlog.log(cam.right, false);
			wlog.log(L"\n", false);
			float ft_avg = ft_total/cnt;
			std::wstring frametimestr = L"FPS avg: " + 
				std::to_wstring(1e6L/ft_avg) + L"\t" +
				L"Frametime avg: "+std::to_wstring(ft_avg)+L"µs\n";
			wlog.log(frametimestr);
			cnt=0;
			ft_total=0.L;
		}
		start=end;

		if(glfwGetKey(win, GLFW_KEY_Q)) {
			cam.orientation = glm::rotate(
				cam.orientation, -1*fts_float, camera::_direction
			);
			cam.direction = cam.orientation * camera::_direction * 
				glm::conjugate(cam.orientation) * fts_float;
			cam.up        = cam.orientation * camera::_up  * 
				glm::conjugate(cam.orientation) * fts_float;
			cam.right     = cam.orientation * camera::_right  * 
				glm::conjugate(cam.orientation) * fts_float;
		}
		if(glfwGetKey(win, GLFW_KEY_E)) {
			cam.orientation = glm::rotate(
				cam.orientation,  1*fts_float, camera::_direction
			);
			cam.direction = cam.orientation * camera::_direction  * 
				glm::conjugate(cam.orientation) * fts_float;
			cam.up        = cam.orientation * camera::_up  * 
				glm::conjugate(cam.orientation) * fts_float;
			cam.right     = cam.orientation * camera::_right  * 
				glm::conjugate(cam.orientation) * fts_float;
		}
		if(glfwGetKey(win, GLFW_KEY_W)) {
			cam.orientation = glm::rotate(
				cam.orientation,  1*fts_float, camera::_right
			);
			cam.direction = cam.orientation * camera::_direction  * 
				glm::conjugate(cam.orientation) * fts_float;
			cam.up        = cam.orientation * camera::_up  * 
				glm::conjugate(cam.orientation) * fts_float;
			cam.right     = cam.orientation * camera::_right  * 
				glm::conjugate(cam.orientation) * fts_float;
		}
		if(glfwGetKey(win, GLFW_KEY_S)) {
			cam.orientation = glm::rotate(
				cam.orientation,  -1*fts_float, camera::_right
			);
			cam.direction = cam.orientation * camera::_direction  * 
				glm::conjugate(cam.orientation) * fts_float;
			cam.up        = cam.orientation * camera::_up * 
				glm::conjugate(cam.orientation) * fts_float;
			cam.right     = cam.orientation * camera::_right  * 
				glm::conjugate(cam.orientation) * fts_float;
		}
		if(glfwGetKey(win, GLFW_KEY_A)) {
			cam.orientation = glm::rotate(
				cam.orientation,  1*fts_float, camera::_up
			);
			cam.direction = cam.orientation * camera::_direction * 
				glm::conjugate(cam.orientation) * fts_float;
			cam.up        = cam.orientation * camera::_up * 
				glm::conjugate(cam.orientation) * fts_float;
			cam.right     = cam.orientation * camera::_right  * 
				glm::conjugate(cam.orientation) * fts_float;
		}
		if(glfwGetKey(win, GLFW_KEY_D)) {
			cam.orientation = glm::rotate(
				cam.orientation,  -1*fts_float, camera::_up
			);
			cam.direction = cam.orientation * camera::_direction * 
				glm::conjugate(cam.orientation) * fts_float;
			cam.up        = cam.orientation * camera::_up * 
				glm::conjugate(cam.orientation) * fts_float;
			cam.right     = cam.orientation * camera::_right  * 
				glm::conjugate(cam.orientation) * fts_float;
		}
		cam.orientation = glm::normalize(cam.orientation);

		if(glfwGetKey(win, GLFW_KEY_UP)) {
		 	cam.position += glm::normalize(cam.direction)*fts_float*100.f;
		}
		if(glfwGetKey(win, GLFW_KEY_DOWN)) {
		 	cam.position += -glm::normalize(cam.direction)*fts_float*100.f;
		}
		if(glfwGetKey(win, GLFW_KEY_RIGHT)) {
			cam.position += glm::normalize(cam.right)*fts_float*100.f;
		}
		if(glfwGetKey(win, GLFW_KEY_LEFT)) {
			cam.position += -glm::normalize(cam.right)*fts_float*100.f;
		}
		if(glfwGetKey(win, GLFW_KEY_SPACE)) {
			cam.position += glm::normalize(cam.up)*fts_float*100.f;
		}
		if(glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL)) {
			cam.position += -glm::normalize(cam.up)*fts_float*100.f;
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
			glDrawArrays(GL_POINTS, 0, chunks_x*chunks_y);
		glEndTransformFeedback();
		glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

		glDisable(GL_RASTERIZER_DISCARD);

		glGetQueryObjectuiv(fc_query, GL_QUERY_RESULT, &fc_primitives);

		glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, fc_primitives*sizeof(int)*3, visible_indices);

		glUseProgram(render_program);

		view = glm::lookAt(
			cam.position, cam.position + cam.direction*10.f, cam.up*10.f
		);

		glUniformMatrix4fv(view_uni, 1, GL_FALSE, glm::value_ptr(view));
		glUniform1i(render_spritesheet_uni, 0);


		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_render);
		glViewport(0.f, 0.f, render_size_x, render_size_y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		for(unsigned int i = 0; i < fc_primitives; ++i) {
			unsigned int x = visible_indices[i].x;
			unsigned int y = visible_indices[i].y;
			glm::mat4 transform;
			transform = glm::translate(
				transform,
				glm::vec3(chunk::chunk_size)*
				glm::vec3(chunks[x][y].position)
			);
			glUniformMatrix4fv(
				model_uni, 1, GL_FALSE, glm::value_ptr(transform)
			);
			glBindVertexArray(chunks[x][y].vtx_array);
			glBindBuffer(GL_ARRAY_BUFFER, chunks[x][y].buffer_geometry);
			glDrawArrays(GL_TRIANGLES, 0, chunks[x][y].vertex_count);
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

	delete chunk::offsets;
	glDeleteTextures(1, &empty_chunk.texid);
	delete empty_chunk.IDs;
	for(unsigned int x = 0; x < chunks.size(); ++x) {
		for(unsigned int y = 0; y < chunks[x].size(); ++y) {
			glDeleteTextures(1, &(chunks[x][y].texid));
			glDeleteBuffers(1, &(chunks[x][y].buffer_geometry));
			glDeleteVertexArrays(1, &(chunks[x][y].vtx_array));
			delete chunks[x][y].IDs;
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
