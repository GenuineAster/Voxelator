#version 430

layout(invocations = 6) in;

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

layout(std430, binding=0) buffer buff { bool facesused[6][]; };

out vec3 gTexcoords;
out vec3 gNormal;
out vec3 gPos;
// Input texture to get block ID from
uniform isampler3D IDTex;
// Textures for chunks surrounding the chunk we're generating
//   0: +z, 1: -x, 2: -y, 3: +x, 4: +y, 5: -z
uniform isampler3D neighbors[6];
// Whether the chunk is on the bottom of the world
//  If it is, we don't need to render the bottom of the chunk,
//    as it will never get seen.
uniform bool chunkIsBottom=false;
// Size of chunks
uniform vec3 chunkSize;

// Helper function that gets the block ID from the ID texture
int getID(ivec3 pos) {
	return texelFetch(IDTex, pos, 0).r;
}

int getID(isampler3D chunk, ivec3 pos) {
	return texelFetch(chunk, pos, 0).r;
}

const vec3 normals[6] = {
	vec3( 0, 0,-1),
	vec3(-1, 0, 0),
	vec3( 0,-1, 0),
	vec3( 1, 0, 0),
	vec3( 0, 1, 0),
	vec3( 0, 0, 1),
};

const ivec3 inormals[6] = {
	ivec3( 0, 0,-1),
	ivec3(-1, 0, 0),
	ivec3( 0,-1, 0),
	ivec3( 1, 0, 0),
	ivec3( 0, 1, 0),
	ivec3( 0, 0, 1),
};

const vec2 tex_offsets0[6] = {
	vec2(0,0),
	vec2(0,0),
	vec2(0,0),
	vec2(1,1),
	vec2(1,1),
	vec2(1,1),
};
const vec2 tex_offsets1[6] = {
	vec2(1,0),
	vec2(0,1),
	vec2(0,1),
	vec2(0,1),
	vec2(0,1),
	vec2(0,1),
};
const vec2 tex_offsets2[6] = {
	vec2(0,1),
	vec2(1,0),
	vec2(1,0),
	vec2(1,0),
	vec2(1,0),
	vec2(1,0),
};
const vec2 tex_offsets3[6] = {
	vec2(1,1),
	vec2(1,1),
	vec2(1,1),
	vec2(0,0),
	vec2(0,0),
	vec2(0,0),
};


const vec4 pos_offsets0[6] = {
	vec4(0,0,0,0),
	vec4(0,0,0,0),
	vec4(0,0,0,0),
	vec4(1,1,1,0),
	vec4(1,1,1,0),
	vec4(1,1,1,0),
};
const vec4 pos_offsets1[6] = {
	vec4(1,0,0,0),
	vec4(0,1,0,0),
	vec4(0,0,1,0),
	vec4(1,1,0,0),
	vec4(0,1,1,0),
	vec4(1,0,1,0),
};
const vec4 pos_offsets2[6] = {
	vec4(0,1,0,0),
	vec4(0,0,1,0),
	vec4(1,0,0,0),
	vec4(1,0,1,0),
	vec4(1,1,0,0),
	vec4(0,1,1,0),
};
const vec4 pos_offsets3[6] = {
	vec4(1,1,0,0),
	vec4(0,1,1,0),
	vec4(1,0,1,0),
	vec4(1,0,0,0),
	vec4(0,1,0,0),
	vec4(0,0,1,0),
};

const ivec3 border_check[6] = {
	ivec3(0, 0, 1),
	ivec3(1, 0, 0),
	ivec3(0, 1, 0),
	ivec3(1, 0, 0),
	ivec3(0, 1, 0),
	ivec3(0, 0, 1),
};

const ivec3 border_check_mask[6] = {
	ivec3(1, 1, 0),
	ivec3(0, 1, 1),
	ivec3(1, 0, 1),
	ivec3(0, 1, 1),
	ivec3(1, 0, 1),
	ivec3(1, 1, 0),
};

const ivec3 border_chunk_check_mask[6] = {
	ivec3(0, 0, 0),
	ivec3(0, 0, 0),
	ivec3(0, 0, 0),
	ivec3(1, 0, 0),
	ivec3(0, 1, 0),
	ivec3(0, 0, 1),
};

const ivec3 border_chunk_check_mask2[6] = {
	ivec3(0, 0, 1),
	ivec3(1, 0, 0),
	ivec3(0, 1, 0),
	ivec3(0, 0, 0),
	ivec3(0, 0, 0),
	ivec3(0, 0, 0),
};

const ivec3 border_chunk_check_sum[6] = {
	ivec3( 0, 0, 0),
	ivec3( 0, 0, 0),
	ivec3( 0, 0, 0),
	ivec3(-1, 0, 0),
	ivec3( 0,-1, 0),
	ivec3( 0, 0,-1),
};

const ivec3 border_chunk_check_sum2[6] = {
	ivec3( 0, 0,-1),
	ivec3(-1, 0, 0),
	ivec3( 0,-1, 0),
	ivec3( 0, 0, 0),
	ivec3( 0, 0, 0),
	ivec3( 0, 0, 0),
};


void main() {
	// Get the base position
	vec4 pos = gl_in[0].gl_Position;
	// Convert position to integer vector to be used as identifier
	ivec3 pos_index = ivec3(pos);

	int ID = getID(pos_index);

	//If the block is air, skip it
	if(ID==0)
		return;

	bool border_block, generate_face;

	int n;


	///////// +z
	n = gl_InvocationID;
	// If the block is not touching air, don't render it
	border_block = pos_index*border_check[n]==chunkSize*border_chunk_check_mask[n]+border_chunk_check_sum[n];
	generate_face = border_block&&(getID(neighbors[n], ivec3(pos_index*border_check_mask[n]+chunkSize*border_chunk_check_mask2[n]+border_chunk_check_sum2[n]))==0);
	generate_face = generate_face && !(n==5&&chunkIsBottom&&pos_index.z==chunkSize.z-1);
	generate_face = generate_face ||(!border_block&&(getID(pos_index+inormals[n]) == 0));
	if(generate_face) {
		// Draw first face
		gNormal = normals[n];
		//   First triangle
		gPos = vec3(pos + pos_offsets0[n]);
		gTexcoords = vec3(tex_offsets0[n], ID);
		EmitVertex();
		gPos = vec3(pos + pos_offsets1[n]);
		gTexcoords = vec3(tex_offsets1[n], ID);
		EmitVertex();
		gPos = vec3(pos + pos_offsets2[n]);
		gTexcoords = vec3(tex_offsets2[n], ID);
		EmitVertex();
		//   Second triangle
		gPos = vec3(pos + pos_offsets3[n]);
		gTexcoords = vec3(tex_offsets3[n], ID);
		EmitVertex();
		EndPrimitive();
	}
}
