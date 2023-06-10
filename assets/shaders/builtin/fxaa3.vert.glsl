@include "version.inc.glsl"

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec2 in_TexCoord;

out INTERFACE
{
    vec2 uv;
} Out;

void main()
{
    gl_Position = in_Position;
    Out.uv = in_TexCoord;
}