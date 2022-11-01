layout(location = 0) in vec4 va_position;
layout(location = 1) in vec2 va_texcoord;
layout(location = 2) in vec4 va_normal;
layout(location = 3) in vec4 va_tangent;
layout(location = 4) in vec4 va_color;

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
