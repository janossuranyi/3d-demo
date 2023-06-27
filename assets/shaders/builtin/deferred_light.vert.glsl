@include "defs.inc"
@include "vertex_uniforms.inc.glsl"

layout(location = 0) in vec4 in_Position;

out INTERFACE
{
    vec4 positionVS;
} Out;

void main()
{
#ifdef LIGHT_DIR
    gl_Position = in_Position;
    Out.positionVS = g_freqLowVert.invProjectMatrix * in_Position;
#else    
    gl_Position = g_freqHighVert.WVPMatrix * in_Position;
    Out.positionVS = g_freqHighVert.modelViewMatrix * in_Position;
#endif
}