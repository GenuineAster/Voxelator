#version 430

layout(location=0) in vec3 pos;
layout(location=1) in vec3 normal;
layout(location=2) in vec3 texcoords;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;
out vec3 vNormal;
out vec3 vTexcoords;
out vec3 vPosition;

void main()
{
	vNormal = normalize(transpose(inverse(mat3(projection*view*model)))*normal);
	vTexcoords = texcoords;
	gl_Position = (projection*view*model)*vec4(pos, 1.0);
	vPosition = vec3(gl_Position);
}
