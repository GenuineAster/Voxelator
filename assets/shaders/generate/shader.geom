#version 430

layout(points) in;
layout(triangle_strip, max_vertices = 24) out;

out vec2 gTexcoords;
out vec3 gNormal;
// Input texture to get block ID from
uniform sampler3D IDTex;
// Size of chunks (unused for now)
uniform vec3 chunkSize;
// Normalized sprite size (used as multiplier)
uniform vec2 spriteSizeNormalized;

// Helper function that gets the block ID from the ID texture
int getID(ivec3 pos) {
	return int(texelFetch(IDTex, pos, 0).r*255);
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

	// Get the amount of sprites on the x dimension of the sprite texture
	int numSprites = int(1.f/spriteSizeNormalized.x);
	// Get the sprite's ID
	ivec2 TexID=ivec2(int(ID)%numSprites, int(ID)/numSprites);
	// Get the sprite's base (top left) texcoords
	vec2 baseTexcoords = spriteSizeNormalized*TexID;

	// If the block is not touching air, don't render it
	if(pos_index.z != 0 && getID(pos_index+ivec3(0, 0, -1)) == 0) {
		// Draw first face
		gNormal = vec3(0, 0, -1);
		//   First triangle
		gl_Position = pos;
		gTexcoords = baseTexcoords;
		EmitVertex();
		gl_Position = pos + vec4(1.0, 0.0, 0.0, 0.0);
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 0.0);
		EmitVertex();
		gl_Position = pos + vec4(0.0, 1.0, 0.0, 0.0);
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 1.0);
		EmitVertex();
		//   Second triangle
		gl_Position = pos + vec4(1.0, 1.0, 0.0, 0.0);
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 1.0);
		EmitVertex();
		EndPrimitive();
	}

	if(pos_index.x != 0 && getID(pos_index+ivec3(-1, 0, 0)) == 0) {
		// Draw second face
		gNormal = vec3(-1, 0, 0);
		//   First triangle
		gl_Position = pos;
		gTexcoords = baseTexcoords;
		EmitVertex();
		gl_Position = pos + vec4(0.0, 1.0, 0.0, 0.0);
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = pos + vec4(0.0, 0.0, 1.0, 0.0);
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 0.0);
		EmitVertex();
		//   Second triangle
		gl_Position = pos + vec4(0.0, 1.0, 1.0, 0.0);
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 1.0);
		EmitVertex();
		EndPrimitive();
	}

	if(pos_index.y != 0 && getID(pos_index+ivec3(0, -1, 0)) == 0) {
		// Draw third face
		gNormal = vec3(0, -1, 0);
		//   First triangle
		gl_Position = pos;
		gTexcoords = baseTexcoords;
		EmitVertex();
		gl_Position = pos + vec4(0.0, 0.0, 1.0, 0.0);
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = pos + vec4(1.0, 0.0, 0.0, 0.0);
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 0.0);
		EmitVertex();
		//   Second triangle
		gl_Position = pos + vec4(1.0, 0.0, 1.0, 0.0);
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 1.0);
		EmitVertex();
		EndPrimitive();
	}

	if(pos_index.x != chunkSize.x-1 && getID(pos_index+ivec3(1, 0, 0)) == 0) {
		// Draw fourth face
		gNormal = vec3(1, 0, 0);
		//   First triangle
		gl_Position = pos + vec4(1.0, 1.0, 1.0, 0.0);
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 1.0);
		EmitVertex();
		gl_Position = pos + vec4(1.0, 1.0, 0.0, 0.0);
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = pos + vec4(1.0, 0.0, 1.0, 0.0);
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 0.0);
		EmitVertex();
		//   Second triangle
		gl_Position = pos + vec4(1.0, 0.0, 0.0, 0.0);
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 0.0);
		EmitVertex();
		EndPrimitive();
	}

	if(pos_index.y != chunkSize.y-1 && getID(pos_index+ivec3(0, 1, 0)) == 0) {
		// Draw fifth face
		gNormal = vec3(0, 1, 0);
		//   First triangle
		gl_Position = pos + vec4(1.0, 1.0, 1.0, 0.0);
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 1.0);
		EmitVertex();
		gl_Position = pos + vec4(0.0, 1.0, 1.0, 0.0);
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = pos + vec4(1.0, 1.0, 0.0, 0.0);
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 0.0);
		EmitVertex();
		//   Second triangle
		gl_Position = pos + vec4(0.0, 1.0, 0.0, 0.0);
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 0.0);
		EmitVertex();
		EndPrimitive();
	}

	if(pos_index.z != chunkSize.z-1 && getID(pos_index+ivec3(0, 0, 1)) == 0) {
		// Draw sixth face
		gNormal = vec3(0, 0, 1);
		//   First triangle
		gl_Position = pos + vec4(1.0, 1.0, 1.0, 0.0);
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 1.0);
		EmitVertex();
		gl_Position = pos + vec4(1.0, 0.0, 1.0, 0.0);
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = pos + vec4(0.0, 1.0, 1.0, 0.0);
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 0.0);
		EmitVertex();
		//   Second triangle
		gl_Position = pos + vec4(0.0, 0.0, 1.0, 0.0);
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 0.0);
		EmitVertex();
		EndPrimitive();
	}
}
