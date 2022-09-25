#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aNormal;
layout(location = 2) in vec4 aTangent;
layout(location = 3) in vec4 aColor;
layout(location = 4) in vec2 aTexcoord;

out INTERFACE {
	vec3	worldPos;
	vec2	texcoord;
	vec3	normal;
	vec3	tanWorldPos;
	vec3	tanViewPos;
	vec3	tanLightPos;
	vec4	color;
} Out;

#include <camera>
#include <matrix>

uniform vec4 ligth_position;

void main()
{

	mat3 mNormal3 = mat3(m_Normal);
	vec3 T = normalize( mNormal3 * vec3(aTangent) );
	vec3 N = normalize( mNormal3 * aNormal );
	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	// then retrieve perpendicular vector B with the cross product of T and N
	vec3 B = normalize(cross(N, T) * aTangent.w);

	mat3 TBN = transpose(mat3(T, B, N)); 
	vec4 hPosition = vec4(aPosition, 1.0);

	Out.worldPos	= (m_W * hPosition).xyz;
	Out.normal		= TBN * aNormal.xyz;
	Out.tanLightPos = TBN * light_position.xyz;
	Out.tanViewPos	= TBN * cam_position.xyz;
	Out.tanWorldPos	= TBN * Out.worldPos;
	Out.color		= aColor;
	Out.texcoord	= aTexcoord;

	gl_Position = m_WVP * aPosition;

} 
