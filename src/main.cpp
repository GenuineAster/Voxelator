#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Logger/Logger.hpp>
#include <thread>
#include <sstream>
#include <fstream>
#include <iostream>
#include <locale>
#include <chrono>
#include <algorithm>
#include <array>
#include "ext/stb/stb_image.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using coord_type = uint8_t;

struct block{
    coord_type x, y, z;
    block(int x=0, int y=0, int z=0):x(x),y(y),z(z){}
};

constexpr int block_offset_x = 0;
constexpr int block_offset_y = block_offset_x + sizeof(coord_type);
constexpr int block_offset_z = block_offset_y + sizeof(coord_type);

using block_id = uint8_t;

constexpr float pi = 3.14159;

Logger<wchar_t> wlog{std::wcout};

int cleanup(int rtval, std::wstring extra=L"");
bool readfile(const char* filename, std::string &contents);
bool process_gl_errors();

int main()
{
    using namespace std::literals::chrono_literals;
    wlog.log(L"Starting up.\n");
    wlog.log(L"Initializing GLFW.\n");
    if(!glfwInit())
        return cleanup(-1);
    wlog.log(L"Creating window.\n");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    GLFWwindow *win = glfwCreateWindow(640, 480, "Voxelator!", nullptr, nullptr);
    if(!win)
        return cleanup(-2);

    glfwMakeContextCurrent(win);

    wlog.log(L"Initializing GLEW.\n");
    glewExperimental = GL_TRUE;
    if(glewInit())
        return cleanup(-3);

    process_gl_errors();

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
    
    wlog.log(L"Creating vertex shader.\n");
    std::string shader_vert_source;
    if(!readfile("assets/shaders/shader.vert", shader_vert_source)) {
        return cleanup(-4, L"assets/shaders/shader.vert");
    }
    GLuint shader_vert = glCreateShader(GL_VERTEX_SHADER);
    const char* src = shader_vert_source.c_str();
    glShaderSource(shader_vert, 1, &src, NULL);
    glCompileShader(shader_vert);
    GLint status;
    glGetShaderiv(shader_vert, GL_COMPILE_STATUS, &status);
    char buff[512];
    glGetShaderInfoLog(shader_vert, 512, NULL, buff);
    if(buff[0] && status == GL_TRUE) {
        wlog.log(L"Vertex shader log:\n");
        wlog.log(buff);
    }
    else if(status != GL_TRUE) {
        return cleanup(-5, std::wstring(buff[0], buff[511]));
    }
    process_gl_errors();

    wlog.log(L"Creating fragment shader.\n");
    std::string shader_frag_source;
    if(!readfile("assets/shaders/shader.frag", shader_frag_source)) {
        return cleanup(-4, L"assets/shaders/shader.frag");
    }
    GLuint shader_frag = glCreateShader(GL_FRAGMENT_SHADER);
    src = shader_frag_source.c_str();
    glShaderSource(shader_frag, 1, &src, NULL);
    glCompileShader(shader_frag);
    glGetShaderiv(shader_frag, GL_COMPILE_STATUS, &status);
    glGetShaderInfoLog(shader_frag, 512, NULL, buff);
    if(buff[0] && status == GL_TRUE) {
        wlog.log(L"Fragment shader log:\n");
        wlog.log(buff);
        wlog.log(L"\n");
    }
    else if(status != GL_TRUE) {
        return cleanup(-5, std::wstring(buff[0], buff[511]));
    }

    process_gl_errors();

    wlog.log(L"Creating geometry shader.\n");
    std::string shader_geom_source;
    if(!readfile("assets/shaders/shader.geom", shader_geom_source)) {
        return cleanup(-4, L"assets/shaders/shader.geom");
    }
    GLuint shader_geom = glCreateShader(GL_GEOMETRY_SHADER);
    src = shader_geom_source.c_str();
    glShaderSource(shader_geom, 1, &src, NULL);
    glCompileShader(shader_geom);
    glGetShaderiv(shader_geom, GL_COMPILE_STATUS, &status);
    glGetShaderInfoLog(shader_geom, 512, NULL, buff);
    if(buff[0] && status == GL_TRUE) {
        wlog.log(L"Geometry shader log:\n");
        wlog.log(buff);
        wlog.log(L"\n");
    }
    else if(status != GL_TRUE) {
        wlog.log(buff);
        return cleanup(-5, std::wstring(buff[0], buff[511]));
    }
    process_gl_errors();

    wlog.log(L"Creating and linking shader program.\n");
    GLuint program = glCreateProgram();
    glAttachShader(program, shader_vert);
    glAttachShader(program, shader_frag);
    glAttachShader(program, shader_geom);
    glBindFragDataLocation(program, 0, "outCol");
    glLinkProgram(program);
    glUseProgram(program);

    process_gl_errors();

    wlog.log(L"Loading spritesheet.\n");
    glActiveTexture(GL_TEXTURE0);
    GLuint spritesheet_tex;
    glGenTextures(1, &spritesheet_tex);
    glBindTexture(GL_TEXTURE_2D, spritesheet_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    float col[] = { 0.0f, 1.0f, 0.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, col);
    int spritesheet_x, spritesheet_y, spritesheet_n;
    unsigned char *spritesheet_data = stbi_load(
        "assets/images/spritesheet.png", &spritesheet_x, &spritesheet_y, &spritesheet_n, 4
    );
    wlog.log(L"Spritesheet size: {" + std::to_wstring(spritesheet_x) + L", " + std::to_wstring(spritesheet_y) + L"}, " + std::to_wstring(spritesheet_n) + L"cpp\n");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, spritesheet_x, spritesheet_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, spritesheet_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(spritesheet_data);
    glm::vec2 spritesheet_size(spritesheet_x, spritesheet_y);
    glm::vec2 sprite_size(16, 16);
    glm::vec2 sprite_size_normalized = sprite_size/spritesheet_size;
    glm::ivec2 n_sprites = spritesheet_size/sprite_size;

    process_gl_errors();

    constexpr int x=64,y=64,z=64,total=x*y*z;
    const int tx=n_sprites.x,ty=n_sprites.y;
    wlog.log(L"Creating ");
    wlog.log(std::to_wstring(total), false);
    wlog.log(L" blocks.\n", false);
    std::array<block,    total> *blocks    = new std::array<block,    total>;
    std::generate(blocks->begin(), blocks->end(), [_x=x,_y=y]{
        static uint8_t x,y,z=y=x=0;
        static bool first=true;
        if(!first){
            if(x>=_x-1){x=0;++y;}
            else ++x;
            if(y>=_y){y=0;++z;}
        } else first=false;
        return block{x,y,z};
    });

    wlog.log(L"Generating Vertex Buffer Object.\n");
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(*blocks), blocks->data(), GL_STATIC_DRAW);
    process_gl_errors();

    wlog.log(L"Creating Chunk Info Texture.\n");
    std::array<block_id, total> *block_ids = new std::array<block_id, total>;
    std::generate(block_ids->begin(), block_ids->end(), [_tx=tx,_ty=ty]{
    	static uint8_t t=0;
    	t+=2;
    	if(t==_tx*_ty)
    		t=0;
    	else if(t>_tx*_ty)
    		t=1;
        // wlog.log(L"Generating " + std::to_wstring(t) + L"\n");
        return t;
    });
    glActiveTexture(GL_TEXTURE1);
    GLuint chunk_info_tex;
    glGenTextures(1, &chunk_info_tex);
    glBindTexture(GL_TEXTURE_3D, chunk_info_tex);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, col);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, x, y, z, 0, GL_RED, GL_UNSIGNED_BYTE, block_ids->data());
    glGenerateMipmap(GL_TEXTURE_3D);
    process_gl_errors();

    wlog.log(L"Creating and getting transform uniform data.\n");
    glm::mat4 transform;
    transform = glm::translate(transform, glm::vec3(-x, -y, -z));
    transform = glm::rotate(transform, pi/4.f, glm::vec3(0.0f, 0.0f, 1.0f));
    GLint transform_uni = glGetUniformLocation(program, "transform");
    glUniformMatrix4fv(transform_uni, 1, GL_FALSE, glm::value_ptr(transform));

    process_gl_errors();


    wlog.log(L"Creating and getting camera direction uniform data.\n");
    glm::vec3 camera_pos = glm::vec3(x, y, 0.0f);
    glm::vec3 camera_target = glm::vec3(0.f, 0.f, 0.0f);
    glm::vec3 camera_dir = glm::normalize(camera_target-camera_pos);
    GLint camera_dir_uni = glGetUniformLocation(program, "cameraDir");
    glUniform3fv(camera_dir_uni, 1, glm::value_ptr(camera_pos));
    process_gl_errors();

    wlog.log(L"Creating and getting view uniform data.\n");
    glm::mat4 view = glm::lookAt(
        camera_pos,
        camera_target,
        glm::vec3(-1.0f, 0.0f, 0.0f)
    );
    GLint view_uni = glGetUniformLocation(program, "view");
    glUniformMatrix4fv(view_uni, 1, GL_FALSE, glm::value_ptr(view));

    process_gl_errors();


    wlog.log(L"Creating and getting projection uniform data.\n");
    glm::mat4 projection = glm::perspective(pi/3.f, 640.0f / 480.0f, 0.01f, 1000.0f);
    GLint projection_uni = glGetUniformLocation(program, "projection");
    glUniformMatrix4fv(projection_uni, 1, GL_FALSE, glm::value_ptr(projection));

    process_gl_errors();

    wlog.log(L"Creating and setting normalized sprite size uniform data.\n");
    GLint sprite_size_uni = glGetUniformLocation(program, "spriteSizeNormalized");
    glUniform2fv(sprite_size_uni, 1, glm::value_ptr(sprite_size_normalized));

	process_gl_errors();

    wlog.log(L"Creating and setting spritesheet texture uniform data.\n");
    GLint spritesheet_uni = glGetUniformLocation(program, "spritesheet");
    glUniform1i(spritesheet_uni, 0);

    process_gl_errors();

    wlog.log(L"Creating and setting block chunk texture uniform data.\n");
    GLint chunk_id_uni = glGetUniformLocation(program, "IDTex");
    glUniform1i(chunk_id_uni, 1);

    process_gl_errors();

    wlog.log(L"Creating and setting block chunk size uniform data.\n");
    GLint chunk_size_uni = glGetUniformLocation(program, "chunkSize");
    glUniform3fv(chunk_size_uni, 1, glm::value_ptr(glm::vec3(x,y,z)));

    process_gl_errors();

    wlog.log(L"Setting position vertex attribute data.\n");
    GLint pos_attrib = glGetAttribLocation(program, "pos");
    glEnableVertexAttribArray(pos_attrib);
    glVertexAttribPointer(pos_attrib, 3, GL_UNSIGNED_BYTE, GL_FALSE, 3, 0);

    process_gl_errors();

    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now(), end = start, timetoprint = start;

    glFlush();

    long long cnt=0;
    float ft_total=0.f;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    while(!glfwWindowShouldClose(win)) {
        end = std::chrono::high_resolution_clock::now();
        long int ft = (std::chrono::duration_cast<std::chrono::microseconds>(end-start).count());
        ft_total += ft;
        ++cnt;
        if(std::chrono::duration_cast<std::chrono::seconds>(end-timetoprint).count() >= 1) {
            timetoprint = std::chrono::high_resolution_clock::now();
            float ft_avg = ft_total/cnt;
            std::wstring frametimestr = L"FPS avg: "+std::to_wstring(1e6f/ft_avg) + L"\t" L"Frametime avg: "+std::to_wstring(ft_avg)+L"µs\n";
            wlog.log(frametimestr);
            cnt=0;
            ft_total=0.f;
        }
        start=end;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        transform = glm::rotate(transform, ft/3e6f, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(transform_uni, 1, GL_FALSE, glm::value_ptr(transform));
        glDrawArrays(GL_POINTS, 0, total);
        glfwSwapBuffers(win);
        glfwPollEvents();
        process_gl_errors();
        // std::this_thread::sleep_for(16ms);
    }

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
