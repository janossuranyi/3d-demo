#include "version.inc.glsl"

layout(location = 0) in vec4 va_position;

out INTERFACE {
    vec4 TexCoords;
} Out;

uniform mat4 m_P;
uniform mat4 m_V;

void main()
{
    Out.TexCoords = va_position;
    vec4 pos = m_P * m_V * va_position;

    gl_Position = pos.xyww;
}
