@include "version.inc.glsl"
@include "defs.inc"
@include "fragment_uniforms.inc.glsl"
@include "common.inc.glsl"

layout(binding = IMU_HDR)       uniform sampler2D tHDRAccum;
layout(binding = IMU_DIFFUSE)   uniform sampler2D tDiffuse;
layout(binding = IMU_EMMISIVE)  uniform sampler2D tBloom;
layout(binding = IMU_AORM)      uniform sampler2D tAO;

out vec3 fragColor0;
in vec2 texcoord;

void main()
{
    vec2 texCoord = gl_FragCoord.xy * g_freqLowFrag.screenSize.zw;
    float occlusion = texture(tAO, texCoord).x;
    vec3 color = texture(tHDRAccum, texCoord).xyz;
    vec3 bloom = texture(tBloom, texCoord).xyz;
    vec3 ambient = texture(tDiffuse, texCoord).xyz * g_freqLowFrag.ambientColor.xyz * g_freqLowFrag.ambientColor.w;
    color += bloom * g_freqLowFrag.bloomParams.y + ambient * occlusion;
    color *= gExposure;

    fragColor0 = GammaIEC( tonemap_Reinhard( color ) );
}