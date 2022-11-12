#include "version.inc.glsl"

layout(location = 0) in vec4 in_Position;
layout(location = 5) in uvec4 in_Flags;

uniform samplerBuffer g_vData;

out INTERFACE {
    vec4 TexCoords;
    flat uint flags;
    flat vec4 vdata;
} Out;

uniform mat4 m_P;
uniform mat4 m_V;

void main()
{
    Out.vdata = texelFetch(g_vData, 100);
    Out.flags = uint(in_Flags.x);
    Out.TexCoords = in_Position;
    vec4 pos = m_P * m_V * in_Position;

    gl_Position = pos.xyww;
}
