@include "version.inc.glsl"
@include "defs.inc"
@include "vertex_uniforms.inc.glsl"
@include "common_uniforms.inc.glsl"

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec2 in_TexCoord;

out INTERFACE
{
    vec2 texCoord;
    vec4 positionVS;
} Out;

void main()
{
    gl_Position = in_Position;
    Out.texCoord = in_TexCoord * 2.0;
    Out.positionVS = g_freqLowVert.invProjectMatrix * gl_Position;
}