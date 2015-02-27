#version 430

layout(points) in;
layout(triangle_strip, max_vertices = 15) out;

in  mat4 vTransform[];
in  mat4 vProjection[];
in  mat4 vView[];
in  vec2 vTexID[];
in  vec2 vSpriteSizeNormalized[];
in  vec3 vCameraDir[];
out vec2 gTexcoords;

void main() {
	mat4 preEndTrans = vView[0]*vTransform[0];
	mat4 endTrans = vProjection[0] * vView[0] * vTransform[0];
	vec2 baseTexcoords = vSpriteSizeNormalized[0]*vTexID[0];
	vec3 cam2tri = normalize(vec3(preEndTrans*gl_in[0].gl_Position)-vec3(vView[0]*vec4(vCameraDir[0], 0.0)));
	vec3 normal;
	float check;

	// Draw first face
	normal = normalize(vec3(preEndTrans*vec4(0.0, 0.0, -1.0, 0.0)));
	check  = dot(cam2tri, normal);
	if(check < 0) {
		//   First triangle
		gl_Position = endTrans * (gl_in[0].gl_Position);
		gTexcoords = baseTexcoords;
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 0.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 0.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 1.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 1.0);
		EmitVertex();
		//   Second triangle
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 0.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 0.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 1.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 1.0);
		EmitVertex();
		EndPrimitive();
	}
	// Draw second face
	normal = normalize(vec3(preEndTrans*vec4(-1.0, 0.0, 0.0, 0.0)));
	check  = dot(cam2tri, normal);
	if(check < 0) {
		//   First triangle
		gl_Position = endTrans * (gl_in[0].gl_Position);
		gTexcoords = baseTexcoords;
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 1.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 0.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 0.0);
		EmitVertex();
		//   Second triangle
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 1.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 1.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 1.0);
		EmitVertex();
		EndPrimitive();
	}
	// Draw third face
	normal = normalize(vec3(preEndTrans*vec4(0.0, -1.0, 0.0, 0.0)));
	check  = dot(cam2tri, normal);
	if(check < 0) {
		//   First triangle
		gl_Position = endTrans * (gl_in[0].gl_Position);
		gTexcoords = baseTexcoords;
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 0.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 0.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 0.0);
		EmitVertex();
		//   Second triangle
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 0.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 0.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 1.0);
		EmitVertex();
		EndPrimitive();
	}
	// Draw fourth face
	normal = normalize(vec3(preEndTrans*vec4(1.0, 0.0, 0.0, 0.0)));
	check  = dot(cam2tri, normal);
	if(check < 0) {
		// //   First triangle
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 1.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 1.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 0.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 0.0);
		EmitVertex();
		//   Second triangle
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 1.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 0.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 0.0);
		EmitVertex();
		EndPrimitive();
	}
	// Draw fifth face
	normal = normalize(vec3(preEndTrans*vec4(0.0, 1.0, 0.0, 0.0)));
	check  = dot(cam2tri, normal);
	if(check < 0) {
		//   First triangle
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 1.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 1.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 1.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 0.0);
		EmitVertex();
		//   Second triangle
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 1.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 1.0, 0.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 0.0);
		EmitVertex();
		EndPrimitive();
	}
	// Draw sixth face
	normal = normalize(vec3(preEndTrans*vec4(0.0, 0.0, 1.0, 0.0)));
	check  = dot(cam2tri, normal);
	if(check < 0) {
		//   First triangle
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 1.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 0.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 1.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 0.0);
		EmitVertex();
		//   Second triangle
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 0.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 0.0, 1.0, 0.0));
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 0.0);
		EmitVertex();
		EndPrimitive();
	}
}
