@include "version.inc.glsl"
@include "vertex_uniforms.inc.glsl"

layout(location = 0) in vec4 in_Position;

void main()
{
    gl_Position = g_freqHighVert.WVPMatrix * in_Position;
}