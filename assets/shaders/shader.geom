#version 400

layout(points) in;
layout(triangle_strip, max_vertices = 30) out;

in  mat4 vTransform[];
in  mat4 vProjection[];
in  mat4 vView[];
in  vec2 vTexID[];
in  vec2 vSpriteSizeNormalized[];
in  vec3 vCameraDir[];
out vec2 gTexcoords;
// out vec4 gCol;

void main() {
	mat4 endTrans = vProjection[0] * vView[0] * vTransform[0];
	// vec4 pos = endTrans * gl_in[0].gl_Position;
	// vec4 cameradiff=vec4(vCameraPos[0], 0.0)-pos;
	vec2 baseTexcoords = vSpriteSizeNormalized[0]*vTexID[0];
	// gCol = vec4(0.5, 0.7, 0.0, 1.0);
	// Draw first face
	//vec4 normal;
	vec3 normal = normalize(vec3(endTrans*vec4(0.0, 0.0, -1.0, 0.0)));
	float check  = dot(vCameraDir[0], normal);
	if(check >= 0) {
		//   First triangle
		gl_Position = endTrans * (gl_in[0].gl_Position);
		// gCol = vec4(0.5, 0.7, 0.0, 1.0);
		gTexcoords = baseTexcoords;
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 0.0, 0.0, 0.0));
		// gCol = vec4(0.0, 0.5, 0.7, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 0.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 1.0, 0.0, 0.0));
		// gCol = vec4(0.7, 0.0, 0.5, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 1.0);
		EmitVertex();
		//   Second triangle
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 0.0, 0.0, 0.0));
		// gCol = vec4(0.5, 0.7, 0.0, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 0.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 1.0, 0.0, 0.0));
		// gCol = vec4(0.0, 0.5, 0.7, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 1.0);
		EmitVertex();
		EndPrimitive();
	}
	// Draw second face
	normal = normalize(vec3(endTrans*vec4(-1.0, 0.0, 0.0, 0.0)));
	check  = dot(vCameraDir[0], normal);
	if(check >= 0) {
		//   First triangle
		gl_Position = endTrans * (gl_in[0].gl_Position);
		// gCol = vec4(0.0, 0.5, 0.7, 1.0);
		gTexcoords = baseTexcoords;
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 1.0, 0.0, 0.0));
		// gCol = vec4(0.7, 0.0, 0.5, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 0.0, 1.0, 0.0));
		// gCol = vec4(0.5, 0.7, 0.0, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 0.0);
		EmitVertex();
		//   Second triangle
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 1.0, 0.0, 0.0));
		// gCol = vec4(0.0, 0.5, 0.7, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 1.0, 1.0, 0.0));
		// gCol = vec4(0.7, 0.0, 0.5, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 1.0);
		EmitVertex();
		EndPrimitive();
	}
	// Draw third face
	normal = normalize(vec3(endTrans*vec4(0.0, -1.0, 0.0, 0.0)));
	check  = dot(vCameraDir[0], normal);
	if(check >= 0) {
		//   First triangle
		gl_Position = endTrans * (gl_in[0].gl_Position);
		// gCol = vec4(0.0, 0.5, 0.7, 1.0);
		gTexcoords = baseTexcoords;
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 0.0, 1.0, 0.0));
		// gCol = vec4(0.7, 0.0, 0.5, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 0.0, 0.0, 0.0));
		// gCol = vec4(0.5, 0.7, 0.0, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 0.0);
		EmitVertex();
		//   Second triangle
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 0.0, 1.0, 0.0));
		// gCol = vec4(0.0, 0.5, 0.7, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 0.0, 1.0, 0.0));
		// gCol = vec4(0.7, 0.0, 0.5, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 1.0);
		EmitVertex();
		EndPrimitive();
	}
	
	// // Draw fourth face
	normal = normalize(vec3(endTrans*vec4(1.0, 0.0, 0.0, 0.0)));
	check  = dot(vCameraDir[0], normal);
	if(check >= 0) {
		// //   First triangle
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 1.0, 1.0, 0.0));
		// gCol = vec4(0.0, 0.5, 0.7, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 1.0, 0.0, 0.0));
		// gCol = vec4(0.7, 0.0, 0.5, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 0.0, 1.0, 0.0));
		// gCol = vec4(0.5, 0.7, 0.0, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 0.0);
		EmitVertex();
		//   Second triangle
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 1.0, 0.0, 0.0));
		// gCol = vec4(0.0, 0.5, 0.7, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 0.0, 0.0, 0.0));
		// gCol = vec4(0.7, 0.0, 0.5, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 0.0);
		EmitVertex();
		EndPrimitive();
	}
	// Draw fifth face
	normal = normalize(vec3(endTrans*vec4(0.0, 1.0, 0.0, 0.0)));
	check  = dot(vCameraDir[0], normal);
	if(check >= 0) {
		//   First triangle
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 1.0, 1.0, 0.0));
		// gCol = vec4(0.0, 0.5, 0.7, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 1.0, 1.0, 0.0));
		// gCol = vec4(0.7, 0.0, 0.5, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 1.0, 0.0, 0.0));
		// gCol = vec4(0.5, 0.7, 0.0, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 0.0);
		EmitVertex();
		//   Second triangle
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 1.0, 1.0, 0.0));
		// gCol = vec4(0.0, 0.5, 0.7, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 1.0, 0.0, 0.0));
		// gCol = vec4(0.7, 0.0, 0.5, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 0.0);
		EmitVertex();
		EndPrimitive();
	}
	// Draw sixth face
	normal = normalize(vec3(endTrans*vec4(0.0, 0.0, 1.0, 0.0)));
	check  = dot(vCameraDir[0], normal);
	if(check >= 0) {
		//   First triangle
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 1.0, 1.0, 0.0));
		// gCol = vec4(0.0, 0.5, 0.7, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 0.0, 1.0, 0.0));
		// gCol = vec4(0.7, 0.0, 0.5, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 1.0, 1.0, 0.0));
		// gCol = vec4(0.5, 0.7, 0.0, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(1.0, 0.0);
		EmitVertex();
		//   Second triangle
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(1.0, 0.0, 1.0, 0.0));
		// gCol = vec4(0.0, 0.5, 0.7, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 1.0);
		EmitVertex();
		gl_Position = endTrans * (gl_in[0].gl_Position + vec4(0.0, 0.0, 1.0, 0.0));
		// gCol = vec4(0.7, 0.0, 0.5, 1.0);
		gTexcoords = baseTexcoords+vSpriteSizeNormalized[0]*vec2(0.0, 0.0);
		EmitVertex();
		EndPrimitive();
	}
}
