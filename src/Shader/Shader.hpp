#ifndef SHADER_HEADER
#define SHADER_HEADER

#include <GL/gl.h>
#include <string>

class Shader
{
private:
	GLuint m_shader;
	GLuint m_type;
public:
	operator GLuint();
	operator bool();
	void create(GLenum type);
	void load_file(GLenum type, std::string file);
	void load_src(GLenum type, std::string src);
	void set_src(std::string src);
	void set_file(std::string src);
	void compile();

	Shader();
	Shader(GLenum type);
	~Shader();
};

#endif