#version 430

layout(depth_unchanged) out float gl_FragDepth;
layout(early_fragment_tests) in;

in vec2 vTexcoords;
in mat4 inverseProjection;

uniform sampler2D normalsTex;
uniform sampler2D colorTex;
uniform sampler2D depthTex;


out vec4 outCol;

//Taken from http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
highp float rand(vec2 co)
{
    highp float a = 12.9898;
    highp float b = 78.233;
    highp float c = 43758.5453;
    highp float dt= dot(co.xy ,vec2(a,b));
    highp float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}

vec4 depth_to_world(vec2 screenspace, float depth) {
	vec4 position;
	position.xy = screenspace;
	position.z  = depth*2.0-1.0;
	position.w  = 1.0;
	position = inverseProjection * position;
	position /= position.w;
	position.z = -position.z;
	return position;
}

vec3 get_position(vec2 uv)
{
	return depth_to_world(uv*2.0-1.0, texture(depthTex, uv).r).xyz;
}

vec2 get_random(vec2 uv)
{
	return vec2(rand(uv.xy), rand(uv.yx));
}

float calc_ao(vec2 tcoord,vec2 uv, vec3 p, vec3 cnorm)
{
	const float intensity = 1.0;
	const float scale = 0.5;
	const float bias = 0.2;
	vec3 diff = get_position(tcoord + uv) - p;
	const vec3 v = normalize(diff);
	const float d = length(diff)*scale;
	return max(0.0,dot(cnorm,v)-bias)*(1.0/(1.0+d))*intensity;
}


void main()
{
	// Discard empty fragments
	float depth = texture(depthTex, vTexcoords).r;
	if(depth >= 0.9999999) {
		discard;
	}

	vec3 Position;
	vec4 position = depth_to_world(vTexcoords.xy * 2.0 - 1.0, depth);
	Position = position.xyz;

	// Get normal Z from X and Y
	vec3 Normal = vec3(texture(normalsTex, vTexcoords));
	Normal.z = -sqrt(1-(Normal.x*Normal.x + Normal.y*Normal.y));

	float ao_sample_rad = 0.3;

	const vec2 vec[4] = {vec2(1,0),vec2(-1,0), vec2(0,1),vec2(0,-1)};

	vec2 r = get_random(vTexcoords);

	float ao = 0.0f;
	float rad = ao_sample_rad/Position.z;

	int iterations = 3;
	for (int j = 0; j < iterations; ++j)
	{
		vec2 coord1 = reflect(vec[j],r)*rad;
		vec2 coord2 = vec2(coord1.x*0.707 - coord1.y*0.707, coord1.x*0.707 + coord1.y*0.707);
		ao += calc_ao(vTexcoords,coord1*0.25, Position, Normal);
		ao += calc_ao(vTexcoords,coord2*0.5, Position, Normal);
		ao += calc_ao(vTexcoords,coord1*0.75, Position, Normal);
		ao += calc_ao(vTexcoords,coord2, Position, Normal);
	}
	ao /= iterations*4.0;

	// Light position is 0,0,0, where the camera is
	vec3 toSurface = normalize(-Position);

	// Brightness is the dot product of the camera to primitive vector and the normal
	float brightness = dot(Normal, toSurface);
	brightness = clamp(brightness, 0.0, 1.0);

	// Mix colors
	outCol = texture(colorTex, vTexcoords);
	outCol.rgb *= 1.0-ao;
	outCol.rgb *= brightness;
}
