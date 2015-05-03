#version 430

layout(location=0) in vec2 pos;
layout(location=1) in vec2 texcoords;

uniform mat4 projection;

out vec2 vTexcoords;
out mat4 inverseProjection;
out mat4 proj;

void main()
{
	proj = projection;
	inverseProjection = inverse(projection);
	vTexcoords = texcoords;
	gl_Position = vec4(pos, 0.0, 1.0);
}
