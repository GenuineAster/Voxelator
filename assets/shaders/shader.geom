#version 430

layout(points) in;
layout(triangle_strip, max_vertices = 15) out;

out vec2 gTexcoords;

uniform sampler3D IDTex;
uniform vec3 chunkSize;
uniform vec2 spriteSizeNormalized;
uniform vec3 cameraDir;
uniform mat4 transform;
uniform mat4 projection;
uniform mat4 view;

int getID(ivec3 pos) {
	return int(texelFetch(IDTex, pos, 0).r*255);
}

void main() {
	vec4 pos = gl_in[0].gl_Position;

	int ID = getID(ivec3(pos));
	int numSprites = int(1.f/spriteSizeNormalized.x);
	ivec2 TexID=ivec2(int(ID)%4, int(ID)/4);

	vec2 baseTexcoords = spriteSizeNormalized*TexID;
	
	mat4 preEndTrans = view*transform;
	mat4 endTrans = projection * view * transform;
	
	vec3 cam2tri = normalize(vec3(preEndTrans*pos)-vec3(view*vec4(cameraDir, 0.0)));

	vec3 normal;
	float check;

	// Draw first face
	normal = normalize(vec3(preEndTrans*vec4(0.0, 0.0, -1.0, 0.0)));
	check  = dot(cam2tri, normal);
	if(check < 0) {
		//   First triangle
		gl_Position = endTrans * (pos);
		gTexcoords = baseTexcoords;
		EmitVertex();
		gl_Position = endTrans * (pos + vec4(1.0, 0.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 0.0);
		EmitVertex();
		gl_Position = endTrans * (pos + vec4(0.0, 1.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 1.0);
		EmitVertex();
		//   Second triangle
		gl_Position = endTrans * (pos + vec4(1.0, 0.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 0.0);
		EmitVertex();
		gl_Position = endTrans * (pos + vec4(1.0, 1.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 1.0);
		EmitVertex();
		EndPrimitive();
	}
	// Draw second face
	normal = normalize(vec3(preEndTrans*vec4(-1.0, 0.0, 0.0, 0.0)));
	check  = dot(cam2tri, normal);
	if(check < 0) {
		//   First triangle
		gl_Position = endTrans * (pos);
		gTexcoords = baseTexcoords;
		EmitVertex();
		gl_Position = endTrans * (pos + vec4(0.0, 1.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (pos + vec4(0.0, 0.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 0.0);
		EmitVertex();
		//   Second triangle
		gl_Position = endTrans * (pos + vec4(0.0, 1.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (pos + vec4(0.0, 1.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 1.0);
		EmitVertex();
		EndPrimitive();
	}
	// Draw third face
	normal = normalize(vec3(preEndTrans*vec4(0.0, -1.0, 0.0, 0.0)));
	check  = dot(cam2tri, normal);
	if(check < 0) {
		//   First triangle
		gl_Position = endTrans * (pos);
		gTexcoords = baseTexcoords;
		EmitVertex();
		gl_Position = endTrans * (pos + vec4(0.0, 0.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (pos + vec4(1.0, 0.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 0.0);
		EmitVertex();
		//   Second triangle
		gl_Position = endTrans * (pos + vec4(0.0, 0.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (pos + vec4(1.0, 0.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 1.0);
		EmitVertex();
		EndPrimitive();
	}
	// Draw fourth face
	normal = normalize(vec3(preEndTrans*vec4(1.0, 0.0, 0.0, 0.0)));
	check  = dot(cam2tri, normal);
	if(check < 0) {
		// //   First triangle
		gl_Position = endTrans * (pos + vec4(1.0, 1.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (pos + vec4(1.0, 1.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (pos + vec4(1.0, 0.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 0.0);
		EmitVertex();
		//   Second triangle
		gl_Position = endTrans * (pos + vec4(1.0, 1.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (pos + vec4(1.0, 0.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 0.0);
		EmitVertex();
		EndPrimitive();
	}
	// Draw fifth face
	normal = normalize(vec3(preEndTrans*vec4(0.0, 1.0, 0.0, 0.0)));
	check  = dot(cam2tri, normal);
	if(check < 0) {
		//   First triangle
		gl_Position = endTrans * (pos + vec4(1.0, 1.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (pos + vec4(0.0, 1.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (pos + vec4(1.0, 1.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 0.0);
		EmitVertex();
		//   Second triangle
		gl_Position = endTrans * (pos + vec4(0.0, 1.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (pos + vec4(0.0, 1.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 0.0);
		EmitVertex();
		EndPrimitive();
	}
	// Draw sixth face
	normal = normalize(vec3(preEndTrans*vec4(0.0, 0.0, 1.0, 0.0)));
	check  = dot(cam2tri, normal);
	if(check < 0) {
		//   First triangle
		gl_Position = endTrans * (pos + vec4(1.0, 1.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (pos + vec4(1.0, 0.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (pos + vec4(0.0, 1.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(1.0, 0.0);
		EmitVertex();
		//   Second triangle
		gl_Position = endTrans * (pos + vec4(1.0, 0.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (pos + vec4(0.0, 0.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+spriteSizeNormalized*vec2(0.0, 0.0);
		EmitVertex();
		EndPrimitive();
	}
}
