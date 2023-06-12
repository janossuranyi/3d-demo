@include "version.inc.glsl"

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec2 in_TexCoord;

out vec2 texCoord;

void main()
{
    gl_Position = in_Position;
    texCoord = in_TexCoord;
}