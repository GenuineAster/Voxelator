#version 430

layout(location=0) in vec3 pos;
layout(location=1) in vec3 normal;
layout(location=2) in vec3 texcoords;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;
out vec3 vNormal;
out vec3 vTexcoords;

void main()
{
	vNormal = normal;
	vTexcoords = texcoords;
	gl_Position = (projection*view*model)*vec4(pos, 1.0);
}
