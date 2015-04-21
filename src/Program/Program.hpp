#ifndef PROGRAM_HEADER
#define PROGRAM_HEADER

#include <GL/gl.h>
#include <string>
#include <vector>
#include <Shader/Shader.hpp>

class Program
{
private:
	GLuint m_program;
public:
	operator GLuint();
	void attach(Shader &shader);
	void link();
	void use();
	void transform_feedback_varyings(std::vector<std::string> varyings, GLenum attrib_mode = GL_INTERLEAVED_ATTRIBS);

	Program();
	~Program();
};

#endif