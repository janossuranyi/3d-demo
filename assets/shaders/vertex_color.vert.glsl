#version 450 core

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec2 in_TexCoord;
layout(location = 2) in vec3 in_Normal;
layout(location = 3) in vec4 in_Tangent;
layout(location = 4) in vec4 in_Color;

out INTERFACE
{
    vec4 fragPos;
    vec4 color;
    vec4 normal;
} Out;

layout(binding = 0, std140) uniform uboUniforms
{
    mat4 localToWorldMatrix;
    mat4 worldToViewMatrix;
    mat4 worldToviewProjectionMatrix;
    vec4 viewOrigin;
};

void main()
{
    gl_Position = in_Position * worldToviewProjectionMatrix * localToWorldMatrix;

    Out.fragPos = in_Position * localToWorldMatrix;
    Out.color = in_Color;
    Out.normal = vec4( in_Normal, 0.0 ) * mat4( transpose( mat3( localToWorldMatrix ) ) );
}