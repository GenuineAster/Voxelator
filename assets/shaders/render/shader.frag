#version 430

layout(depth_unchanged) out float gl_FragDepth;
layout(early_fragment_tests) in;

in vec3 vNormal;
in vec3 vPosition;
in vec3 vTexcoords;
out vec4 outNormal;
out vec4 outColor;

uniform sampler2DArray spritesheet;

void main()
{
	outNormal = vec4(vNormal, 1.0);
	outColor = texture(spritesheet, vTexcoords);
}
