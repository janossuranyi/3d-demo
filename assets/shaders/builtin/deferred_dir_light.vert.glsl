@include "version.inc.glsl"
@include "defs.inc"
@include "vertex_uniforms.inc.glsl"

layout(location = 0) in vec4 in_Position;

out INTERFACE
{
    vec4 positionVS;
    flat mat4 modelViewMtx;
} Out;

void main()
{
    Out.positionVS = g_freqLowVert.invProjectMatrix * in_Position;
    Out.modelViewMtx = gViewMatrix;
    gl_Position = in_Position;
}