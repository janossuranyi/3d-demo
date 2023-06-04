@include "version.inc.glsl"
@include "vertex_uniforms.inc.glsl"

layout(location = 0) in vec4 in_Position;

out INTERFACE
{
    vec4 positionVS;
} Out;

void main()
{
    gl_Position = g_freqHighVert.WVPMatrix * in_Position;
    Out.positionVS = g_freqLowVert.viewMatrix * g_freqHighVert.localToWorldMatrix * in_Position;
}