#include "version.inc.glsl"

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec2 in_TexCoord;
layout(location = 2) in vec3 in_PackedInputs;

uint asuint(float x)
{
    return floatBitsToUint(x);
}
vec4 unpackRGBA8(uint value)
{
    return vec4(float((value >> uint(24)) & 255u), float((value >> uint(16)) & 255u), float((value >> uint(8)) & 255u), float(value & 255u)) / vec4(255.0);
}


uniform mat4 g_mWorldViewProjection;
uniform mat4 g_mWorldTransform;
uniform vec4 g_vViewPosition;
uniform mat3 g_mNormalTransform;

out INTERFACE {
    vec4 normal;
    vec4 tangent;
    vec4 position;
    vec4 texcoord;
    vec4 color;
} Out;

void main()
{
    vec4 localPosition;
    vec4 localNormal;
    vec4 localTangent;
    vec4 vtxCol;
    {
        localPosition           = in_Position;
        localNormal             = normalize( unpackRGBA8( asuint( in_PackedInputs.x ) ).wzyx * 2.0 - 1.0 );
        vec4 unpacked_tangent   = unpackRGBA8( asuint( in_PackedInputs.y ) ).wzyx;
        localTangent.xyz        = normalize( unpacked_tangent.xyz * 2.0 - 1.0 );
        localTangent.xyz        = normalize( localTangent.xyz - dot( localTangent.xyz, localNormal.xyz ) * localNormal.xyz );
        localTangent.w          = floor( unpacked_tangent.w * 255.1 / 128.0 ) * 2.0 - 1.0;
		vtxCol                  = unpackRGBA8( asuint( in_PackedInputs.z ) ).wzyx;
    };

    gl_Position = g_mWorldViewProjection * localPosition ;

    Out.normal    = vec4(g_mNormalTransform * localNormal.xyz, 0.0);
    Out.tangent   = vec4(g_mNormalTransform * localTangent.xyz, localTangent.w);
    Out.position  = vec4(g_mNormalTransform * localPosition.xyz, 1.0) - g_vViewPosition;
    Out.texcoord  = in_TexCoord.xyxy;
    Out.color     = vtxCol;
}

