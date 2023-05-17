@include "version.inc.glsl"
@include "common.inc.glsl"
@include "uniforms.inc.glsl"

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec2 in_TexCoord;
layout(location = 2) in vec3 in_Normal;
layout(location = 3) in vec4 in_Tangent;
layout(location = 4) in vec4 in_Color;

out INTERFACE
{
    vec4 fragPos;
    vec2 texCoord;
    vec4 color;
    vec4 tangent;
    vec3 normal;
} Out;

void main()
{
    gl_Position = ubo.WVPMatrix * in_Position;
    mat3 mNormal3   = mat3(ubo.normalMatrix); // transpose( mat3( ubo.localToWorldMatrix ) ) ;
    //mat3 mModel3    = mat3(ubo.localToWorldMatrix);

    Out.fragPos     = ubo.localToWorldMatrix * in_Position;
    Out.color       = in_Color;
	
    vec4 localTangent = in_Tangent * 2.0 - 1.0;
    //localTangent.w = floor( in_Tangent.w * 255.1 / 128.0 ) * 2.0 - 1.0;

    vec3 T = normalize( mNormal3 * localTangent.xyz );
	vec3 N = normalize( mNormal3 * (in_Normal * 2.0 - 1.0) );

	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);

	// then retrieve perpendicular vector B with the cross product of T and N
	// vec3 B = normalize(cross(N, T) * in_Tangent.w);

    Out.normal      = N;
    Out.tangent     = vec4(T, localTangent.w);
    Out.texCoord    = in_TexCoord;
}