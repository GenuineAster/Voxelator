#version 430

layout(points) in;
layout(triangle_strip, max_vertices = 24) out;

out vec3 gTexcoords;
out vec3 gNormal;
out vec3 gPos;
// Input texture to get block ID from
uniform isampler3D IDTex;
// Textures for chunks surrounding the chunk we're generating
//   0: +x, 1: +y, 2: +z, 3: -x, 4: -y, 5: -z
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

	///////// +z
	// If the block is not touching air, don't render it
	border_block = pos_index.z==0;
	generate_face = border_block&&(getID(neighbors[2], ivec3(pos_index.xy, chunkSize.z-1))==0);
	generate_face = generate_face ||(!border_block&&(getID(pos_index+ivec3(0, 0, -1)) == 0));
	if(generate_face) {
		// Draw first face
		gNormal = vec3(0, 0, -1);
		//   First triangle
		gPos = vec3(pos);
		gTexcoords = vec3(0.f, 0.f, ID);
		EmitVertex();
		gPos = vec3(pos + vec4(1.0, 0.0, 0.0, 0.0));
		gTexcoords = vec3(1.0, 0.0, ID);
		EmitVertex();
		gPos = vec3(pos + vec4(0.0, 1.0, 0.0, 0.0));
		gTexcoords = vec3(0.0, 1.0, ID);
		EmitVertex();
		//   Second triangle
		gPos = vec3(pos + vec4(1.0, 1.0, 0.0, 0.0));
		gTexcoords = vec3(1.0, 1.0, ID);
		EmitVertex();
		EndPrimitive();
	}

	///////// -x
	border_block = pos_index.x==0;
	generate_face = border_block&&(getID(neighbors[3], ivec3(chunkSize.x-1, pos_index.yz))==0);
	generate_face = generate_face ||(!border_block&&(getID(pos_index+ivec3(-1, 0, 0)) == 0));
	if(generate_face) {
		// Draw second face
		gNormal = vec3(-1, 0, 0);
		//   First triangle
		gPos = vec3(pos);
		gTexcoords = vec3(0.f, 0.f, ID);
		EmitVertex();
		gPos = vec3(pos + vec4(0.0, 1.0, 0.0, 0.0));
		gTexcoords = vec3(0.0, 1.0, ID);
		EmitVertex();
		gPos = vec3(pos + vec4(0.0, 0.0, 1.0, 0.0));
		gTexcoords = vec3(1.0, 0.0, ID);
		EmitVertex();
		//   Second triangle
		gPos = vec3(pos + vec4(0.0, 1.0, 1.0, 0.0));
		gTexcoords = vec3(1.0, 1.0, ID);
		EmitVertex();
		EndPrimitive();
	}

	/////////// -y
	border_block = pos_index.y==0;
	generate_face = border_block&&(getID(neighbors[4], ivec3(pos_index.x, chunkSize.y-1, pos_index.z))==0);
	generate_face = generate_face ||(!border_block&&(getID(pos_index+ivec3(0, -1, 0)) == 0));
	if(generate_face) {
		// Draw third face
		gNormal = vec3(0, -1, 0);
		//   First triangle
		gPos = vec3(pos);
		gTexcoords = vec3(0.f, 0.f, ID);
		EmitVertex();
		gPos = vec3(pos + vec4(0.0, 0.0, 1.0, 0.0));
		gTexcoords = vec3(0.0, 1.0, ID);
		EmitVertex();
		gPos = vec3(pos + vec4(1.0, 0.0, 0.0, 0.0));
		gTexcoords = vec3(1.0, 0.0, ID);
		EmitVertex();
		//   Second triangle
		gPos = vec3(pos + vec4(1.0, 0.0, 1.0, 0.0));
		gTexcoords = vec3(1.0, 1.0, ID);
		EmitVertex();
		EndPrimitive();
	}

	//////// +x
	border_block = pos_index.x==chunkSize.x-1;
	generate_face = border_block&&(getID(neighbors[0], ivec3(0, pos_index.yz))==0);
	generate_face = generate_face ||(!border_block&&(getID(pos_index+ivec3(1, 0, 0)) == 0));
	if(generate_face) {
		// Draw fourth face
		gNormal = vec3(1, 0, 0);
		//   First triangle
		gPos = vec3(pos + vec4(1.0, 1.0, 1.0, 0.0));
		gTexcoords = vec3(1.0, 1.0, ID);
		EmitVertex();
		gPos = vec3(pos + vec4(1.0, 1.0, 0.0, 0.0));
		gTexcoords = vec3(0.0, 1.0, ID);
		EmitVertex();
		gPos = vec3(pos + vec4(1.0, 0.0, 1.0, 0.0));
		gTexcoords = vec3(1.0, 0.0, ID);
		EmitVertex();
		//   Second triangle
		gPos = vec3(pos + vec4(1.0, 0.0, 0.0, 0.0));
		gTexcoords = vec3(0.0, 0.0, ID);
		EmitVertex();
		EndPrimitive();
	}

	////// +y
	border_block = pos_index.y==chunkSize.y-1;
	generate_face = border_block&&(getID(neighbors[1], ivec3(pos_index.x, 0, pos_index.z))==0);
	generate_face = generate_face ||(!border_block&&(getID(pos_index+ivec3(0, 1, 0)) == 0));
	if(generate_face) {
		// Draw fifth face
		gNormal = vec3(0, 1, 0);
		//   First triangle
		gPos = vec3(pos + vec4(1.0, 1.0, 1.0, 0.0));
		gTexcoords = vec3(1.0, 1.0, ID);
		EmitVertex();
		gPos = vec3(pos + vec4(0.0, 1.0, 1.0, 0.0));
		gTexcoords = vec3(0.0, 1.0, ID);
		EmitVertex();
		gPos = vec3(pos + vec4(1.0, 1.0, 0.0, 0.0));
		gTexcoords = vec3(1.0, 0.0, ID);
		EmitVertex();
		//   Second triangle
		gPos = vec3(pos + vec4(0.0, 1.0, 0.0, 0.0));
		gTexcoords = vec3(0.0, 0.0, ID);
		EmitVertex();
		EndPrimitive();
	}

	//////// -z
	border_block = pos_index.z==chunkSize.z-1;
	generate_face = border_block&&!chunkIsBottom&&(getID(neighbors[5], ivec3(pos_index.xy, 0))==0);
	generate_face = generate_face ||(!border_block&&(getID(pos_index+ivec3(0, 0, 1)) == 0));
	if(generate_face) {
		// Draw sixth face
		gNormal = vec3(0, 0, 1);
		//   First triangle
		gPos = vec3(pos + vec4(1.0, 1.0, 1.0, 0.0));
		gTexcoords = vec3(1.0, 1.0, ID);
		EmitVertex();
		gPos = vec3(pos + vec4(1.0, 0.0, 1.0, 0.0));
		gTexcoords = vec3(0.0, 1.0, ID);
		EmitVertex();
		gPos = vec3(pos + vec4(0.0, 1.0, 1.0, 0.0));
		gTexcoords = vec3(1.0, 0.0, ID);
		EmitVertex();
		//   Second triangle
		gPos = vec3(pos + vec4(0.0, 0.0, 1.0, 0.0));
		gTexcoords = vec3(0.0, 0.0, ID);
		EmitVertex();
		EndPrimitive();
	}
}
