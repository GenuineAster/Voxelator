#version 430

layout(depth_unchanged) out float gl_FragDepth;
layout(early_fragment_tests) in;

in vec2 vTexcoords;
uniform sampler2D normalsTex;
uniform sampler2D positionsTex;
uniform sampler2D colorTex;
uniform sampler2D depthTex;
out vec4 outCol;

void main()
{
	// Discard empty fragments
	float depth = texture(depthTex, vTexcoords).r;
	if(depth > 0.9999999) {
		discard;
	}

	// Get position from position texture (trying to avoid doing this)
	// This method works fine
	vec3 Position = vec3(texture(positionsTex, vTexcoords));

	// Get normal Z from X and Y
	vec3 Normal = vec3(texture(normalsTex, vTexcoords));
	Normal.z = -sqrt(1-(Normal.x*Normal.x + Normal.y*Normal.y));

	// Light position is 0,0,0, where the camera is
	vec3 toSurface = normalize(-Position);

	// Brightness is the dot product of the camera to primitive vector and the normal
	float brightness = dot(Normal, toSurface);
	brightness = clamp(brightness, 0.0, 1.0);

	// Mix colors
	outCol = texture(colorTex, vTexcoords);
	outCol = vec4(brightness * vec3(1.0) * outCol.rgb, outCol.a);
}
