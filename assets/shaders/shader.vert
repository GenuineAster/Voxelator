#version 150

in vec3 pos;
in vec2 texid;
uniform mat4 transform;
uniform mat4 projection;
uniform mat4 view;
uniform vec2 spriteSizeNormalized;
uniform vec3 cameraPos;
out mat4  vTransform;
out mat4  vProjection;
out mat4  vView;
out vec2  vTexID;
out vec2  vSpriteSizeNormalized;
out vec3  vCameraPos;

void main()
{
	vTransform = transform;
	vProjection = projection;
	vView = view;
	vTexID = texid;
	vSpriteSizeNormalized = spriteSizeNormalized;
    gl_Position = vec4(pos, 1.0);
}
