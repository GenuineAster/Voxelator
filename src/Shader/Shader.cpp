#include <GL/glew.h>
#include <GL/gl.h>
#include <fstream>
#include <Shader/Shader.hpp>

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


void Shader::create(GLenum type) {
	m_type = type;
	m_shader = glCreateShader(type);
}

void Shader::load_src(GLenum type, std::string src) {
	this->create(type);
	this->set_src(src);
	this->compile();
	GLint status;
	glGetShaderiv(m_shader, GL_COMPILE_STATUS, &status);
	if(status != GL_TRUE) {
		// TODO: throw exceptions
	}
}

void Shader::load_file(GLenum type, std::string file) {
	std::string src;
	if(readfile(file.c_str(), src)) {
		this->load_src(type, src);
	}
}

void Shader::set_src(std::string src) {
	const char *src_cstr = src.c_str();
	glShaderSource(m_shader, 1, &src_cstr, NULL);
}

void Shader::set_file(std::string file) {
	std::string src;
	readfile(file.c_str(), src);
	this->set_src(src);
}

void Shader::compile() {
	glCompileShader(m_shader);
}

Shader::operator GLuint() {
	return m_shader;
}


Shader::operator bool() {
	return m_shader != static_cast<GLuint>(-1);
}

Shader::Shader() {
	m_shader = -1;
}

Shader::~Shader() {
	glDeleteShader(m_shader);
}
