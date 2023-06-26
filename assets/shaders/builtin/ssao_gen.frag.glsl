@include "defs.inc"
@include "fragment_uniforms.inc.glsl"
@include "common_uniforms.inc.glsl"
@include "common.inc.glsl"

layout(binding = IMU_FRAGPOS)   uniform sampler2D tFragPosZ;
layout(binding = IMU_NORMAL)    uniform sampler2D tNormal;
layout(binding = IMU_DEFAULT)   uniform sampler2D tNoise;

out vec4 fragColor0;

const vec2 noiseScale = vec2(g_backendData.params[0].x/4.0, g_backendData.params[0].y/4.0); // screen/2/4

in INTERFACE
{
    vec4 positionVS;
} In;

float GetLinearZ(vec2 uv)
{
    return -textureLod( tFragPosZ, uv, 0 ).x * gFarClipDistance;
}

vec4 reconstructPositionVS(vec3 viewRay, vec2 uv)
{
    float linearZ = textureLod( tFragPosZ, uv, 0 ).x * gFarClipDistance;
    return vec4( viewRay * linearZ, 1.0 );
}

mat3 buildBasis(vec3 n, vec3 r)
{
    vec3 t = normalize( r - n * dot( r, n ) );
    vec3 b = cross( n, t );
    return mat3( t, b, n );
}

vec2 GetSampleTexCoord(vec3 samplePos)
{
    vec4 offset = g_freqLowFrag.projectMatrix * vec4( samplePos, 1.0 );
    offset.xyz /= offset.w;
    offset.xyz = offset.xyz * 0.5 + 0.5;
    return offset.xy;
}

void main()
{
    vec2 UV = screenPosToTexcoord( gl_FragCoord.xy, g_backendData.params[0] );

    vec4 fragPosVS  = reconstructPositionVS( In.positionVS.xyz, UV );
    vec3 normal     = NormalOctDecode( texture( tNormal, UV ).xy, false);
    vec3 randomVec  = texture( tNoise, UV * noiseScale).xyz;

    mat3 TBN = buildBasis( normal, randomVec );

    float occlusion = 0.0;
    float radius = g_backendData.params[1].x;
    float bias = g_backendData.params[1].y;
    float strength = g_backendData.params[1].z;
    const int kernelSize = g_commonData.ssaoKernel.length();

    for(int i = 0; i < kernelSize; ++i)
    {
        vec3 samplePos = TBN * g_commonData.ssaoKernel[ i ].xyz; // from tangent to view-space
        samplePos = samplePos * radius + fragPosVS.xyz;
        float sampleDepth = GetLinearZ( GetSampleTexCoord( samplePos ) );

        //float rangeCheck = smoothstep( 0.0, 1.0, radius / abs( fragPosVS.z - sampleDepth ) );
        float rangeCheck= abs(fragPosVS.z - sampleDepth) < radius ? 1 : 0;
        occlusion += (sampleDepth > samplePos.z + bias ? 1: 0) * rangeCheck;
    }
    occlusion = saturate( 1.0 - ( strength * occlusion / float(kernelSize) ) );

    fragColor0.x = occlusion;
}