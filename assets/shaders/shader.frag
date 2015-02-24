#version 400

// in vec4 gCol;
in vec2 gTexcoords;
out vec4 outCol;

uniform sampler2D tex;

void main()
{
    //outCol = vec4(1.0, 0.0, 0.0, 1.0);
    // outCol=gCol;
    // outCol = (texture(tex, gTexcoords)-texture(tex, gTexcoords))+gCol;
    outCol = texture(tex, gTexcoords);
}
