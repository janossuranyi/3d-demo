@include "version.inc.glsl"
@include "defs.inc"
@include "vertex_uniforms.inc.glsl"

layout(location = 0) in vec4 in_Position;

out INTERFACE
{
    vec4 positionVS;
} Out;

void main()
{
    Out.positionVS = g_freqLowVert.invProjectMatrix * in_Position;
    
    gl_Position = in_Position;
}