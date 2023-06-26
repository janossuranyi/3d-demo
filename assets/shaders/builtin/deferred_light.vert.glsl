@include "defs.inc"
@include "vertex_uniforms.inc.glsl"

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec2 in_TexCoord;

out INTERFACE
{
    vec4 positionVS;
} Out;

void main()
{
    gl_Position = g_freqHighVert.WVPMatrix * in_Position;
#if LIGHT_SPOT_POINT    
    Out.positionVS = g_freqHighVert.modelViewMatrix * in_Position;
#else    
    Out.positionVS = g_freqLowVert.invProjectMatrix * in_Position;
#endif
}