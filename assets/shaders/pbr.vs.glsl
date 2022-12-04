#include "version.inc.glsl"

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec2 in_TexCoord;
layout(location = 2) in vec3 in_PackedInputs;

uint asuint(float x)
{
    return floatBitsToUint(x);
}
vec4 unpackR8G8B8A8(uint value)
{
    return vec4(float((value >> uint(24)) & 255u), float((value >> uint(16)) & 255u), float((value >> uint(8)) & 255u), float(value & 255u)) / vec4(255.0);
}


uniform mat4 g_mWorldViewProjection;
uniform mat4 g_mWorldTransform;
uniform vec4 g_vViewPosition;
uniform mat3 g_mNormalTransform;

out INTERFACE {
   vec3 FragPos;
   vec2 TexCoords;
   vec3 TangentViewPos;
   vec3 TangentFragPos;
   vec3 TangentNormal;
   vec3 TangentLightPos;
   vec4 Color;
   vec3 tangent;
} Out;

struct light_t {
    vec3 pos;
    vec3 color;    
};

uniform light_t g_lights[1];

void main()
{
    vec4 localPosition;
    vec4 localNormal;
    vec4 localTangent;
    vec4 vtxCol;
    {
        localPosition = in_Position;
        localPosition.w = 1.0;
        localNormal = unpackR8G8B8A8( asuint( in_PackedInputs.x ) ).wzyx * 2.0 - 1.0;
        vec4 unpacked_tangent = unpackR8G8B8A8( asuint( in_PackedInputs.y ) ).wzyx;
        localTangent.xyz = unpacked_tangent.xyz * 2.0 - 1.0;
        localTangent.w = floor( unpacked_tangent.w * 255.1 / 128.0 ) * 2.0 - 1.0;
		vtxCol = unpackR8G8B8A8( asuint( in_PackedInputs.z ) ).wzyx;
    };

    vec3 T = normalize( g_mNormalTransform * localTangent.xyz);
    vec3 N = normalize( g_mNormalTransform * localNormal.xyz);
    T = normalize( T - dot( T, N ) * N );
    vec3 B = normalize( cross( N, T ) * localTangent.w);
    mat3 TBN = transpose( mat3( T, B, N ) );

    gl_Position = g_mWorldViewProjection * localPosition ;

    Out.TexCoords = in_TexCoord;
    Out.FragPos = (g_mWorldTransform * localPosition).xyz;
    Out.TangentViewPos = TBN * g_vViewPosition.xyz;
    Out.TangentFragPos = TBN * Out.FragPos;
    Out.TangentNormal = TBN * localNormal.xyz;
    Out.TangentLightPos = TBN * g_lights[0].pos;
    Out.Color = vtxCol;

    Out.tangent = localTangent.xyz;
}

