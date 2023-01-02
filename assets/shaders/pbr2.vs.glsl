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

float dot4 ( vec4 a, vec4 b ) { return dot( a, b ); }

#define MVP_X 0
#define MVP_Y 1
#define MVP_Z 2
#define MVP_W 3
#define N_X 4
#define N_Y 5
#define N_Z 6
uniform vec4 _va_freqHigh[7];

#define VIEW_ORIGIN 0
uniform vec4 _va_freqLow[1];

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

   // memoryBarrier();

    mat4 mvpmatrix = mat4(
        _va_freqHigh[ MVP_X ],
        _va_freqHigh[ MVP_Y ],
        _va_freqHigh[ MVP_Z ],
        _va_freqHigh[ MVP_W ]
    );

    gl_Position = mvpmatrix * localPosition;

//    mat3 normalmatrix = mat3(normal_mtx);
    mat3 normalmatrix = mat3(
        _va_freqHigh[ N_X ].xyz,
        _va_freqHigh[ N_Y ].xyz,
        _va_freqHigh[ N_Z ].xyz
    );

    Out.normal    = vec4(normalmatrix * localNormal.xyz, 0.0);
    Out.tangent   = vec4(normalmatrix * localTangent.xyz, localTangent.w);
    Out.position  = vec4(normalmatrix * localPosition.xyz, 1.0) - _va_freqLow[ VIEW_ORIGIN ];
    Out.texcoord  = in_TexCoord.xyxy;
    Out.color     = vtxCol;
}

