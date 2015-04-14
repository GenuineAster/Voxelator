#version 430

out vec4 outCol;
in vec3 vTexcoords;
in vec3 vNormal;
in vec3 vPosition;

uniform sampler2DArray spritesheet;

void main()
{
	vec3 toSurface = normalize(-vPosition);
	float brightness = dot(vNormal, toSurface);
	brightness = clamp(brightness, 0.0, 1.0);
	outCol = texture(spritesheet, vTexcoords);
	outCol = vec4(brightness * vec3(1.0) * outCol.rgb, outCol.a);
}
