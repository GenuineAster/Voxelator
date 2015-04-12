#version 430

layout(location=0) in vec2 pos;
layout(location=1) in vec2 texcoords;
out vec2 vTexcoords;

void main()
{
	vTexcoords = texcoords;
	gl_Position = vec4(pos, 0.0, 1.0);
}
