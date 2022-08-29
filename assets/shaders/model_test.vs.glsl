#version 450 core

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

layout(std140, binding = 2) uniform cb_camera
{
    vec4 cam_position;
    vec4 cam_target;
    vec4 cam_direction;
    vec4 cam_up;
	float znear;
	float zfar;
	float yfov;
	float ascept;
};

out VS_OUT {
   vec3 FragPos;
   vec2 TexCoords;
   vec3 TangentLightPos;
   vec3 TangentViewPos;
   vec3 TangentFragPos;
} vs_out;

void main()
{
    mat3 mNormal3 = mat3(m_Normal);

    vec3 aTangent = vec3(vTangent);
	vec3 T = normalize( mNormal3 * vec3(aTangent) );
	vec3 N = normalize( mNormal3 * vNormal );
	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	// then retrieve perpendicular vector B with the cross product of T and N
	vec3 B = cross(N, T);

	mat3 TBN = transpose(mat3(T, B, N)); 

    gl_Position = m_WVP * vec4(vPosition, 1.0);
    
    vs_out.TexCoords = vTexCoord;
    vs_out.FragPos = vec3(m_W * vec4(vPosition, 1.0));
    vs_out.TangentLightPos = TBN * vec3(cam_position);
    vs_out.TangentViewPos  = TBN * vec3(cam_position);
    vs_out.TangentFragPos  = TBN * vec3(m_W * vec4(vPosition,1.0));
}

