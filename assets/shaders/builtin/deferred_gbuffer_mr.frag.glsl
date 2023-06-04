@include "version.inc.glsl"
@include "common.inc.glsl"
@include "fragment_uniforms.inc.glsl"

in INTERFACE
{
    vec4 positionVS;
    vec2 texCoord;
    vec4 color;
    vec4 tangent;
    vec3 normal;
} In;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec4 outSpec;
layout(location = 3) out vec4 outFragPos;

layout(binding = IMU_DIFFUSE)   uniform sampler2D tDiffuse;
layout(binding = IMU_NORMAL)    uniform sampler2D tNormal;
layout(binding = IMU_AORM)      uniform sampler2D tAORM;
layout(binding = IMU_EMMISIVE)  uniform sampler2D tEmissive;

struct input_t {
    mat3 tbn;
    vec3 normal;
    vec4 sampleAmbient;
    vec4 samplePBR;
    vec4 spec;
};

const float ONE_OVER_255 = 1.0/255.0;

vec3 ReconstructNormal(vec3 normalTS) { return normalize(normalTS * 2.0 - 1.0); }

float packR8G8B8A8(vec4 v)
{
    v = clamp(v * vec4(255.0), 0.0, 255.0);
    uint i = (asuint(v.w) & 255u) << uint(24) | (asuint(v.z) & 255u) << uint(16) | (asuint(v.y) & 255u) << uint(8) | asuint(v.x) & 255u;
    return asfloat(i);
}

void main()
{
    input_t inputs;
    {
        vec3 localTangent       = normalize( In.tangent.xyz );
        vec3 localNormal        = normalize( In.normal );
        vec3 derivedBitangent   = normalize( cross( localNormal, localTangent ) * In.tangent.w );
        vec3 normalTS           = ReconstructNormal(texture(tNormal, In.texCoord).xyz) * vec3(1.0, -1.0, 1.0);
        inputs.tbn      = mat3(localTangent,derivedBitangent,localNormal);
        inputs.normal   = inputs.tbn * normalTS;
    }

    inputs.sampleAmbient    = g_freqHighFrag.matDiffuseFactor * SRGBlinear( texture( tDiffuse, In.texCoord ) );

    {
        uint params_x = asuint(gFlagsX);
        uint localCoverage = (params_x >> FLG_X_COVERAGE_SHIFT) & FLG_X_COVERAGE_MASK;
        if ( localCoverage == FLG_COVERAGE_MASKED && inputs.sampleAmbient.a < gAlphaCutoff )
        { 
            discard;
        }
    }

    inputs.samplePBR        = texture( tAORM, In.texCoord ).yzxw * vec4(gRoughnessFactor, gMetallicFactor, 1.0, 1.0);

    outFragPos.x = -In.positionVS.z / gFarClipDistance;
    outAlbedo    = inputs.sampleAmbient;
    outSpec      = inputs.samplePBR;
    outNormal    = (1.0 + inputs.normal) * 0.5;
}