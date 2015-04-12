#version 430

out vec4 color;
in vec2 vTexcoords;

uniform sampler2D framebuffer;

void main()
{
	color = texture(framebuffer, vTexcoords);
}
