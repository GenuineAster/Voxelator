#version 430

out vec4 outCol;
in vec2 gTexcoords;

uniform sampler2D spritesheet;

void main()
{
	outCol = texture(spritesheet, gTexcoords);
}
