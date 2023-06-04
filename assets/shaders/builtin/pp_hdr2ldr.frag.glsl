@include "version.inc.glsl"
@include "fragment_uniforms.inc.glsl"
@include "common.inc.glsl"

layout(binding = IMU_HDR)       uniform sampler2D tHDRAccum;
layout(binding = IMU_DIFFUSE)   uniform sampler2D tDiffuse;

out vec3 fragColor;

void main()
{
    vec2 texCoord = gl_FragCoord.xy * g_freqLowFrag.screenSize.zw;
    vec3 color = texture(tHDRAccum, texCoord).xyz;
    vec3 ambient = texture(tDiffuse, texCoord).xyz * g_freqLowFrag.ambientColor.xyz * g_freqLowFrag.ambientColor.w;
    color += ambient;
    color = vec3(1.0) - exp(-color * gExposure);
    fragColor = GammaIEC(color);
}