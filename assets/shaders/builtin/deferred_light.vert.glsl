@include "version.inc.glsl"
@include "defs.inc"
@include "vertex_uniforms.inc.glsl"

layout(location = 0) in vec4 in_Position;

out INTERFACE
{
    vec3 positionVS;
} Out;

void main()
{
    gl_Position = g_freqHighVert.WVPMatrix * in_Position;
    Out.positionVS = vec3(g_freqLowVert.viewMatrix * g_freqHighVert.localToWorldMatrix * in_Position);
}