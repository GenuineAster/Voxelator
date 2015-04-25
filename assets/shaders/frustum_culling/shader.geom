#version 430

layout(points) in;
layout(points, max_vertices = 1) out;
in vec3 vPos[];

// Size of chunks
uniform vec3 chunkSize;

uniform mat4 view;
uniform mat4 proj;

out ivec3 gPos;

void main() {
	float clipoffset = 22.63;
	mat4 trans = proj*view;
	vec4 p = trans*vec4(vPos[0]*chunkSize+vec3(0.0,0.0,180),1.0);
	vec3 w = p.www+clipoffset;
	bvec3 c1 =    lessThan(p.xyz, -w);
	bvec3 c2 = greaterThan(p.xyz,  w);
	bool condition = !(any(c1) || any(c2));
	if(condition) {
		gPos = ivec3(vPos[0]);
		EmitVertex();
		EndPrimitive();
	}
}
