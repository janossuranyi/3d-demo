@include "version.inc.glsl"
@include "defs.inc"
@include "fragment_uniforms.inc.glsl"
@include "common_uniforms.inc.glsl"

layout(binding = IMU_FRAGPOS)   uniform sampler2D tFragPosZ;
layout(binding = IMU_NORMAL)    uniform sampler2D tNormal;
layout(binding = IMU_DEFAULT)   uniform sampler2D tNoise;

out vec4 fragColor0;

const vec2 noiseScale = vec2(g_backendData.params[0].x/4.0, g_backendData.params[0].y/4.0); // screen/2/4
vec2 screenPosToTexcoord(vec2 pos, vec4 scale) { return pos * scale.zw; }

in INTERFACE
{
    vec4 positionVS;
} In;

float texDepth(sampler2D samp, vec2 uv)
{
    return -textureLod( samp, uv, 0 ).x * gFarClipDistance;
}

vec4 reconstructPositionVS(vec3 viewRay, sampler2D depthTex, vec2 uv)
{
    float linearZ = textureLod( depthTex, uv, 0 ).x * gFarClipDistance;
    return vec4( viewRay * linearZ, 1.0 );
}

float saturate(float x) { return clamp(x, 0.0, 1.0); }

void main()
{
    vec2 UV = screenPosToTexcoord( gl_FragCoord.xy, g_backendData.params[0] );

    vec4 fragPosVS  = reconstructPositionVS( In.positionVS.xyz, tFragPosZ, UV );
    vec3 normal     = texture( tNormal, UV ).xyz;
    vec3 randomVec  = texture( tNoise, UV * noiseScale).xyz;

    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    float radius = g_backendData.params[1].x;
    float bias = g_backendData.params[1].y;
    const int kernelSize = g_commonData.ssaoKernel.length();

    for(int i = 0; i < kernelSize; ++i)
    {
        vec3 samplePos = TBN * g_commonData.ssaoKernel[i].xyz; // from tangent to view-space
        samplePos = samplePos * radius + fragPosVS.xyz;
        vec4 offset = vec4(samplePos,1.0);
        offset = g_freqLowFrag.projectMatrix * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;
        float sampleDepth = texDepth(tFragPosZ, offset.xy);

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPosVS.z - sampleDepth));
        //float rangeCheck= abs(fragPosVS.z - sampleDepth) < radius ? 1.0 : 0.0;
        //occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
        occlusion += mix( 0.0, 1.0, sampleDepth >= samplePos.z + bias );
    }
    occlusion = saturate( 1.0 - ( occlusion / float(kernelSize) ) );

    fragColor0.x = occlusion;
}