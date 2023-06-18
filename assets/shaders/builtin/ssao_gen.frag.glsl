@include "version.inc.glsl"
@include "defs.inc"
@include "fragment_uniforms.inc.glsl"
@include "common_uniforms.inc.glsl"

in INTERFACE
{
    vec2 texCoord;
    vec4 positionVS;
} In;

layout(binding = IMU_FRAGPOS)   uniform sampler2D tFragPosZ;
layout(binding = IMU_NORMAL)    uniform sampler2D tNormal;
layout(binding = IMU_DEFAULT)   uniform sampler2D tNoise;

out vec4 fragColor0;

const vec2 noiseScale = vec2(g_commonData.renderTargetRes.x/4.0, g_commonData.renderTargetRes.y/4.0); // screen

void main()
{
    vec3 viewRay = In.positionVS.xyz;
    float nDepth = -1.0 * texture( tFragPosZ, In.texCoord ).x;
    vec4 fragPosVS = vec4(viewRay * nDepth, 1.0);
    vec3 normal = texture( tNormal, In.texCoord ).xyz;
    vec3 randomVec = texture( tNoise, In.texCoord * noiseScale).xyz;

    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    float radius = 0.5;
    float bias = 0.025;
    int kernelSize = g_commonData.ssaoKernel.length();
    for(int i = 0; i < kernelSize; ++i)
    {
        vec3 samplePos = TBN * g_commonData.ssaoKernel[i].xyz; // from tangent to view-space
        samplePos = fragPosVS.xyz + samplePos * radius;
        vec4 offset = vec4(samplePos,1.0);
        offset = g_freqLowFrag.projectMatrix * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;
        float sampleDepth = -1.0 * texture(tFragPosZ, offset.xy).x ;
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPosVS.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;  
        //occlusion += step(samplePos.z + bias, sampleDepth) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / float(kernelSize));

    fragColor0.x = occlusion;
}