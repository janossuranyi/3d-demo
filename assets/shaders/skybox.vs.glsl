#version 330 core
layout (location = 0) in vec3 vaPosition;

out vec3 vso_TexCoords;

uniform mat4 m_P;
uniform mat4 m_V;

void main()
{
    vso_TexCoords = vaPosition;
    vec4 pos = m_P * m_V * vec4(vaPosition, 1.0);

    gl_Position = pos.xyww;
}
