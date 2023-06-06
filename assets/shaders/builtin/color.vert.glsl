@include "version.inc.glsl"
@include "vertex_uniforms.inc.glsl"

layout(location = 0) in vec4 in_Position;
layout(location = 4) in vec4 in_Color;

out INTERFACE
{
    vec4 color;
} Out;

void main()
{
    gl_Position = g_freqHighVert.WVPMatrix * in_Position;
    Out.color = in_Color;
}