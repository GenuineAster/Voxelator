
// For gnu::unused attribute
#ifndef __GNUC__
	#define gnu::
#endif

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Logger/Logger.hpp"
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

// Common macro for casting OpenGL buffer offsets
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

constexpr float win_size_x = 960;
constexpr float win_size_y = 540;
constexpr float render_size_x = 3840;
constexpr float render_size_y = 2160;

using coord_type = uint8_t;
using block_id = uint8_t;

constexpr GLsizei components_per_vtx = 10;

//Specify amount of chunks
constexpr uint32_t   chunks_x=12;
constexpr uint32_t   chunks_y=12;
constexpr uint32_t   chunks_z=0; // Unused for now
// Specify chunk sizes, chunk_size_*  and chunk_total must be a power of 2.
constexpr coord_type chunk_size_x=64;
constexpr coord_type chunk_size_y=64;
constexpr coord_type chunk_size_z=64;
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
	std::generate(chunk::offsets->begin(), chunk::offsets->end(), []{
		static uint8_t x,y,z=y=x=0;
		static bool first=true;
		if(!first){
			if(x>=chunk_size_x-1){x=0;++y;}
			else ++x;
			if(y>=chunk_size_y){y=0;++z;}
		} else first=false;
		return block{x,y,z};
	});

	using namespace std::literals::chrono_literals;

	wlog.log(L"Starting up.\n");
	wlog.log(L"Initializing GLFW.\n");

	if(!glfwInit())
		return cleanup(-1);

	glfwSetErrorCallback(
		[]([[gnu::unused]] int a, const char* b){
			wlog.log(std::wstring{b, b+std::strlen(b)}+L"\n");
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
		win_size_x, win_size_y, "Voxelator!", nullptr, nullptr
	);

	// If window creation fails, exit.
	if(!win)
		return cleanup(-2);

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

	wlog.log(L"Creating generate vertex shader.\n");
	std::string shader_generate_vert_source;
	if(!readfile("assets/shaders/generate/shader.vert", shader_generate_vert_source)) {
		return cleanup(-4, L"assets/shaders/generate/shader.vert");
	}
	GLuint shader_generate_vert = glCreateShader(GL_VERTEX_SHADER);
	const char* src = shader_generate_vert_source.c_str();
	glShaderSource(shader_generate_vert, 1, &src, NULL);
	glCompileShader(shader_generate_vert);
	GLint status;
	glGetShaderiv(shader_generate_vert, GL_COMPILE_STATUS, &status);
	char buff[512];
	glGetShaderInfoLog(shader_generate_vert, 512, NULL, buff);
	if(buff[0] && status == GL_TRUE) {
		wlog.log(L"Generate vertex shader log:\n");
		wlog.log(buff, false);
		wlog.log(L"\n", false);
	}
	else if(status != GL_TRUE) {
		return cleanup(-5, std::wstring(buff[0], buff[511]));
	}
	process_gl_errors();

	wlog.log(L"Creating generate geometry shader.\n");
	std::string shader_generate_geom_source;
	if(!readfile("assets/shaders/generate/shader.geom", shader_generate_geom_source)) {
		return cleanup(-4, L"assets/shaders/generate/shader.geom");
	}
	GLuint shader_generate_geom = glCreateShader(GL_GEOMETRY_SHADER);
	src = shader_generate_geom_source.c_str();
	glShaderSource(shader_generate_geom, 1, &src, NULL);
	glCompileShader(shader_generate_geom);
	glGetShaderiv(shader_generate_geom, GL_COMPILE_STATUS, &status);
	glGetShaderInfoLog(shader_generate_geom, 512, NULL, buff);
	if(buff[0] && status == GL_TRUE) {
		wlog.log(L"Generate geometry shader log:\n");
		wlog.log(buff, false);
		wlog.log(L"\n", false);
	}
	else if(status != GL_TRUE) {
		wlog.log(buff);
		return cleanup(-5, std::wstring(buff[0], buff[511]));
	}
	process_gl_errors();

	wlog.log(L"Creating and linking generate shader program.\n");
	GLuint generate_program = glCreateProgram();
	glAttachShader(generate_program, shader_generate_vert);
	glAttachShader(generate_program, shader_generate_geom);
	const char *varyings[] = {"gl_Position", "gTexcoords", "gNormal"};
	glTransformFeedbackVaryings(generate_program, 3, varyings, GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(generate_program);
	glUseProgram(generate_program);

	process_gl_errors();
	
	wlog.log(L"Creating render vertex shader.\n");
	std::string shader_render_vert_source;
	if(!readfile("assets/shaders/render/shader.vert", shader_render_vert_source)) {
		return cleanup(-4, L"assets/shaders/render/shader.vert");
	}
	GLuint shader_render_vert = glCreateShader(GL_VERTEX_SHADER);
	src = shader_render_vert_source.c_str();
	glShaderSource(shader_render_vert, 1, &src, NULL);
	glCompileShader(shader_render_vert);
	glGetShaderiv(shader_render_vert, GL_COMPILE_STATUS, &status);
	glGetShaderInfoLog(shader_render_vert, 512, NULL, buff);
	if(buff[0] && status == GL_TRUE) {
		wlog.log(L"Render vertex shader log:\n");
		wlog.log(buff, false);
		wlog.log(L"\n", false);
	}
	else if(status != GL_TRUE) {
		return cleanup(-5, std::wstring(buff[0], buff[511]));
	}
	process_gl_errors();

	wlog.log(L"Creating render fragment shader.\n");
	std::string shader_render_frag_source;
	if(!readfile("assets/shaders/render/shader.frag", shader_render_frag_source)) {
		return cleanup(-4, L"assets/shaders/render/shader.frag");
	}
	GLuint shader_render_frag = glCreateShader(GL_FRAGMENT_SHADER);
	src = shader_render_frag_source.c_str();
	glShaderSource(shader_render_frag, 1, &src, NULL);
	glCompileShader(shader_render_frag);
	glGetShaderiv(shader_render_frag, GL_COMPILE_STATUS, &status);
	glGetShaderInfoLog(shader_render_frag, 512, NULL, buff);
	if(buff[0] && status == GL_TRUE) {
		wlog.log(L"Fragment shader log:\n");
		wlog.log(buff, false);
		wlog.log(L"\n", false);
	}
	else if(status != GL_TRUE) {
		return cleanup(-5, std::wstring(buff[0], buff[511]));
	}

	process_gl_errors();

	wlog.log(L"Creating and linking render shader program.\n");
	GLuint render_program = glCreateProgram();
	glAttachShader(render_program, shader_render_vert);
	glAttachShader(render_program, shader_render_frag);
	glBindFragDataLocation(render_program, 0, "outCol");
	glLinkProgram(render_program);
	glUseProgram(render_program);

	process_gl_errors();

	wlog.log(L"Creating display vertex shader.\n");
	std::string shader_display_vert_source;
	if(!readfile("assets/shaders/display/shader.vert", shader_display_vert_source)) {
		return cleanup(-4, L"assets/shaders/display/shader.vert");
	}
	GLuint shader_display_vert = glCreateShader(GL_VERTEX_SHADER);
	src = shader_display_vert_source.c_str();
	glShaderSource(shader_display_vert, 1, &src, NULL);
	glCompileShader(shader_display_vert);
	glGetShaderiv(shader_display_vert, GL_COMPILE_STATUS, &status);
	glGetShaderInfoLog(shader_display_vert, 512, NULL, buff);
	if(buff[0] && status == GL_TRUE) {
		wlog.log(L"display vertex shader log:\n");
		wlog.log(buff, false);
		wlog.log(L"\n", false);
	}
	else if(status != GL_TRUE) {
		return cleanup(-5, std::wstring(buff[0], buff[511]));
	}
	process_gl_errors();

	wlog.log(L"Creating display fragment shader.\n");
	std::string shader_display_frag_source;
	if(!readfile("assets/shaders/display/shader.frag", shader_display_frag_source)) {
		return cleanup(-4, L"assets/shaders/display/shader.frag");
	}
	GLuint shader_display_frag = glCreateShader(GL_FRAGMENT_SHADER);
	src = shader_display_frag_source.c_str();
	glShaderSource(shader_display_frag, 1, &src, NULL);
	glCompileShader(shader_display_frag);
	glGetShaderiv(shader_display_frag, GL_COMPILE_STATUS, &status);
	glGetShaderInfoLog(shader_display_frag, 512, NULL, buff);
	if(buff[0] && status == GL_TRUE) {
		wlog.log(L"Fragment shader log:\n");
		wlog.log(buff, false);
		wlog.log(L"\n", false);
	}
	else if(status != GL_TRUE) {
		return cleanup(-5, std::wstring(buff[0], buff[511]));
	}

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
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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
			chunks[x][y].tex = 1+i;
			chunks[x][y].texnum = GL_TEXTURE0 + 1 + i;
			chunks[x][y].IDs = new std::array<block_id, chunk_total>;
			chunks[x][y].position = glm::vec3(x, y, 0.f);
			int _x,_y,_z=_y=_x=0;
			bool first=true;
			std::generate(chunks[x][y].IDs->begin(), chunks[x][y].IDs->end(), 
				[&x=_x,&y=_y,&z=_z,n_sprites,&first,&rd_engine,&dist]{
					if(!first){
						if(x>=chunk_size_x-1){x=0;++y;}
						else ++x;
						if(y>=chunk_size_y){y=0;++z;}
					} else first=false;
					int height = abs(x-(chunk_size_x/2)) + 
						abs(y-(chunk_size_y/2));
					return (z>height)?dist(rd_engine):0;
				}
			);
			glActiveTexture(chunks[x][y].texnum);
			glGenTextures(1, &chunks[x][y].texid);
			glBindTexture(GL_TEXTURE_3D, chunks[x][y].texid);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);
			glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, col);
			glTexImage3D(
				GL_TEXTURE_3D, 0, GL_RED, chunk_size_x, chunk_size_y, 
				chunk_size_z, 0, GL_RED, GL_UNSIGNED_BYTE, 
				chunks[x][y].IDs->data()
			);
			glGenerateMipmap(GL_TEXTURE_3D);
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
		pi/3.f, render_size_x/render_size_y, 0.01f, 1000.0f
	);
	GLint projection_uni = glGetUniformLocation(render_program, "projection");
	glUniformMatrix4fv(projection_uni, 1, GL_FALSE, glm::value_ptr(projection));

	process_gl_errors();

	wlog.log(L"Creating and setting normalized sprite size uniform data.\n");
	GLint sprite_size_uni = glGetUniformLocation(
		render_program, "spriteSizeNormalized"
	);
	glUniform2fv(sprite_size_uni, 1, glm::value_ptr(sprite_size_normalized));

	process_gl_errors();

	glUseProgram(render_program);

	std::chrono::high_resolution_clock::time_point start, end, timetoprint;
	timetoprint = end = start = std::chrono::high_resolution_clock::now();

	glFlush();

	long long cnt=0;
	long double ft_total=0.f;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	wlog.log(L"Setting up transform feedback.\n");

	glUseProgram(generate_program);

	wlog.log(L"Creating and setting block chunk texture uniform data.\n");
	GLint chunk_id_uni = glGetUniformLocation(generate_program, "IDTex");
	glUniform1i(chunk_id_uni, 1);

	wlog.log(L"Creating and setting block chunk neighbors uniform data.\n");
	GLint neighbor_id_uni = glGetUniformLocation(generate_program, "neighbors");

	wlog.log(L"Creating and setting block chunk is bottom uniform data.\n");
	GLint chunk_is_bottom_id_uni = glGetUniformLocation(generate_program, "chunkIsBottom");

	wlog.log(L"Creating and setting spritesheet texture uniform data.\n");
	GLint spritesheet_uni = glGetUniformLocation(generate_program, "spritesheet");
	glUniform1i(spritesheet_uni, 0);

	process_gl_errors();

	wlog.log(L"Creating and setting block chunk size uniform data.\n");
	GLint chunk_size_uni = glGetUniformLocation(generate_program, "chunkSize");
	glUniform3fv(chunk_size_uni, 1, glm::value_ptr(
		glm::vec3(chunk_size_x,chunk_size_y,chunk_size_z))
	);

	process_gl_errors();

	GLint gen_sprite_size_uni = glGetUniformLocation(
		generate_program, "spriteSizeNormalized"
	);
	glUniform2fv(gen_sprite_size_uni, 1, glm::value_ptr(sprite_size_normalized));

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
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(float)*chunk_total*12*3*components_per_vtx, nullptr, GL_STATIC_DRAW);

	uint64_t chunk_total_primitives = 0;
	uint64_t chunk_total_vertices = 0;
	uint64_t chunk_total_components = 0;

	GLuint query;
	glGenQueries(1, &query);
	for(unsigned int x=0;x<chunks.size();++x) {
		for(unsigned int y=0;y<chunks[x].size();++y) {
			glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, tbo);
			glEnable(GL_RASTERIZER_DISCARD);
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);

			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBindVertexArray(generate_vao);

			// Get neighboring chunks, or set to empty_chunk if none
			GLint neighbor_tex[6];
			if(x<chunks_x-1)
				neighbor_tex[0]=chunks[x+1][y].tex;
			else
				neighbor_tex[0]=empty_chunk.tex;
			if(y<chunks_y-1)
				neighbor_tex[1]=chunks[x][y+1].tex;
			else
				neighbor_tex[1]=empty_chunk.tex;
			neighbor_tex[2]=empty_chunk.tex;
			if(x)
				neighbor_tex[3]=chunks[x-1][y].tex;
			else
				neighbor_tex[3]=empty_chunk.tex;
			if(y)
				neighbor_tex[4]=chunks[x][y-1].tex;
			else
				neighbor_tex[4]=empty_chunk.tex;
			neighbor_tex[5]=empty_chunk.tex;

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
			glBufferData(GL_COPY_WRITE_BUFFER, sizeof(float)*primitives*3*components_per_vtx, nullptr, GL_STATIC_DRAW);
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
				glVertexAttribPointer(pos_attrib, 4, GL_FLOAT, GL_FALSE, components_per_vtx*sizeof(GLfloat), BUFFER_OFFSET(sizeof(float)*0));
			}

			GLint texcoord_attrib = glGetAttribLocation(render_program, "texcoords");
			if(texcoord_attrib != -1) {
				glEnableVertexAttribArray(texcoord_attrib);
				glVertexAttribPointer(texcoord_attrib, 3, GL_FLOAT, GL_FALSE, components_per_vtx*sizeof(GLfloat), BUFFER_OFFSET(sizeof(float)*4));
			}

			GLint normal_attrib = glGetAttribLocation(render_program, "normal");
			if(normal_attrib != -1) {
				glEnableVertexAttribArray(normal_attrib);
				glVertexAttribPointer(normal_attrib, 3, GL_FLOAT, GL_FALSE, components_per_vtx*sizeof(GLfloat), BUFFER_OFFSET(sizeof(float)*7));
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
				}
			} break;
		}
	});

	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
	// glEnable(GL_CULL_FACE);
	// glFrontFace(GL_CW);
	// glCullFace(GL_BACK);

	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	GLuint framebuffer_texture;
	glGenTextures(1, &framebuffer_texture);
	glActiveTexture(GL_TEXTURE0+3);
	glBindTexture(GL_TEXTURE_2D, framebuffer_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, render_size_x, render_size_y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer_texture, 0);
	GLuint renderbuffer;
	glGenRenderbuffers(1, &renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, render_size_x, render_size_y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);

	float fb_vertices[] = {
		// Coords  Texcoords
		-1.f, -1.f,   0.f, 0.f,
		 1.f, -1.f,   1.f, 0.f,
		-1.f,  1.f,   0.f, 1.f,
		-1.f,  1.f,   0.f, 1.f,
		 1.f, -1.f,   1.f, 0.f,
		 1.f,  1.f,   1.f, 1.f,
	};
	GLuint fb_vbo, fb_vao;
	glGenVertexArrays(1, &fb_vao);
	glBindVertexArray(fb_vao);
	glGenBuffers(1, &fb_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, fb_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fb_vertices), fb_vertices, GL_STATIC_DRAW);

	GLint framebuffer_uni = glGetUniformLocation(display_program, "framebuffer");

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

	glUseProgram(render_program);

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

		glUseProgram(render_program);

		view = glm::lookAt(
			cam.position, cam.position + cam.direction*10.f, cam.up*10.f
		);

		glUniformMatrix4fv(view_uni, 1, GL_FALSE, glm::value_ptr(view));

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glViewport(0.f, 0.f, render_size_x, render_size_y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for(unsigned int x=0;x<chunks.size();++x) {
			for(unsigned int y=0;y<chunks[x].size();++y) {
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
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDisable(GL_DEPTH_TEST);
		glUseProgram(display_program);
		glViewport(0.f, 0.f, win_size_x, win_size_y);

		glBindVertexArray(fb_vao);
		glBindBuffer(GL_ARRAY_BUFFER, fb_vbo);
		
		glUniform1i(framebuffer_uni, 3);
		
		glDrawArrays(GL_TRIANGLES, 0, 6);
		

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

bool readfile(const char* filename, std::string &contents)
{
	std::ifstream f(filename);
	if(!f.good()) {
		return false;
	}
	char c;
	while((c = f.get()),f.good()) {
		contents+=c;
	}
	f.close();
	return true;
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
