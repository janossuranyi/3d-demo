#include "version.inc.glsl"

layout(location = 0) in vec4 in_Position;

out INTERFACE {
    vec4 TexCoords;
} Out;

uniform mat4 m_P;
uniform mat4 m_V;

void main()
{
    Out.TexCoords = in_Position;
    vec4 pos = m_P * m_V * in_Position;

    gl_Position = pos.xyww;
}
