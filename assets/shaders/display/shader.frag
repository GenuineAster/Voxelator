#version 430

out vec4 color;
in vec2 vTexcoords;

uniform sampler2D framebuffer;
uniform vec2 viewport_size = vec2(960.f, 540.f);

void main()
{
	float c = pow(length(vec2(vTexcoords)-vec2(0.5,0.5)),1);
	color = mix(texture(framebuffer, vTexcoords), vec4(0.f), c);
	color.a = 1.f;
}
