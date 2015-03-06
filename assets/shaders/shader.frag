#version 430

layout(early_fragment_tests) in;

out vec4 outCol;
in vec2 gTexcoords;
in float dist;

uniform sampler2D spritesheet;

const float fog_dis = 128.f;
const float fog_len = 512.f;

void main()
{
	outCol = texture(spritesheet, gTexcoords);
	if(dist-fog_dis>0) {outCol = mix(outCol, vec4(0.f, 0.f, 0.f, 1.f), (dist-fog_dis)/fog_len);}
}
