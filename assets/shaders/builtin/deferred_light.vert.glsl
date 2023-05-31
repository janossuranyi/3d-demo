@include "version.inc.glsl"
@include "vertex_uniforms.inc.glsl"

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec2 in_TexCoord;
layout(location = 2) in vec3 in_Normal;
layout(location = 3) in vec4 in_Tangent;
layout(location = 4) in vec4 in_Color;

out INTERFACE
{
    vec2 texCoord;
} Out;

void main()
{
    gl_Position = g_freqHighVert.WVPMatrix * in_Position;
    Out.texCoord = in_TexCoord;
}