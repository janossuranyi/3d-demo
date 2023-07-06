layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec2 in_TexCoord;

out vec2 texCoord;
//const vec2 vertices[3]=vec2[3](vec2(-1,-1), vec2(3,-1), vec2(-1, 3));

void main()
{
    texCoord = in_TexCoord;
    gl_Position = in_Position;
}