#version 430

//layout(early_fragment_tests) in;

out vec4 outCol;
in vec3 vTexcoords;
in vec3 vNormal;
in float dist;

uniform sampler2DArray spritesheet;

const float fog_dis = 128.f;
const float fog_len = 512.f;

void main()
{
	// outCol = vec4(vNormal, 1.0);
	outCol += texture(spritesheet, vTexcoords);// - vec4(vNormal, 1.0);
}
