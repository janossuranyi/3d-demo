#version 450 core

layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec4 aNormal;
layout(location = 2) in vec4 aTangent;
layout(location = 3) in vec4 aColor;
layout(location = 4) in vec2 aTexcoord;

out INTERFACE {
	vec3	worldPosition;
	vec3	normal;
	vec3 	tangent;
	vec3	bitangent;
	vec2	texcoord;
	vec4	color;
} Out;

#include <matrix>

void main()
{
	Out.worldPosition	= m_WVP * aPosition;
	Out.normal			= m_Normal * aNormal;
	Out.tangent			= m_Normal * aTangent;
	Out.color			= aColor;
	Out.texcoord		= aTexcoord;

	gl_Position = Out.worldPosition;

} 
