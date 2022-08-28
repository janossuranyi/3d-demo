#version 430 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec4 vTangent;
layout(location = 3) in vec2 vTexCoord;

layout(std140, binding = 1) uniform cb_matrix
{
    mat4 m_W;
    mat4 m_V;
    mat4 m_P;
    mat4 m_Normal;

    mat4 m_WV;
    mat4 m_VP;
    mat4 m_WVP;
    
    mat4 m_iP;
    mat4 m_iVP;    
};

out VS_OUT {
   vec4 Position;
   vec3 Normal;
} vs_out;

void main()
{
    gl_Position = m_WVP * vec4(vPosition, 1.0);
    vs_out.Position = m_W * vec4(vPosition, 1.0);
    vs_out.Normal =  mat3(m_Normal) * vNormal;
}

