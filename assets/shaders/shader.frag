#version 430

in vec2 gTexcoords;
out vec4 outCol;

uniform sampler2D tex;

void main()
{
	outCol = texture(tex, gTexcoords);
}
