@include "version.inc.glsl"

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec2 in_TexCoord;
layout(location = 2) in vec3 in_Normal;
layout(location = 3) in vec4 in_Tangent;
layout(location = 4) in vec4 in_Color;

out INTERFACE
{
    vec4 fragPos;
    vec4 color;
    vec4 tangent;
    vec3 normal;
} Out;

@include "uniforms.inc.glsl"

void main()
{
    gl_Position = ubo.WVPMatrix * in_Position;
    mat3 mNormal3 = transpose( mat3( ubo.localToWorldMatrix ) ) ;

    Out.fragPos     = ubo.localToWorldMatrix * in_Position;
    Out.color       = in_Color;
	
    vec3 T = normalize( mNormal3 * vec3(in_Tangent) );
	vec3 N = normalize( mNormal3 * in_Normal );

	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);

	// then retrieve perpendicular vector B with the cross product of T and N
	// vec3 B = normalize(cross(N, T) * in_Tangent.w);

    Out.normal      = N;
    Out.tangent     = vec4(T, in_Tangent.w);
}