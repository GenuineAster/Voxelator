#version 430

out vec4 color;
in vec2 vTexcoords;

uniform sampler2D framebuffer;
uniform vec2 viewport_size = vec2(960.f, 540.f);

void main()
{
	color = texture(framebuffer, vTexcoords);
}
