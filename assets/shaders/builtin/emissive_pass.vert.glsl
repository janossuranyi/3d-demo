@include "defs.inc"
@include "vertex_uniforms.inc.glsl"

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec2 in_TexCoord;

out INTERFACE
{
    vec2 texCoord;
} Out;

void main()
{
    Out.texCoord = in_TexCoord;
    gl_Position = g_freqHighVert.WVPMatrix * in_Position;
}