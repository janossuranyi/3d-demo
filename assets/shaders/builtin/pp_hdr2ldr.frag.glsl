@include "version.inc.glsl"
@include "fragment_uniforms.inc.glsl"

#define	IMU_DIFFUSE 0
#define IMU_HDR 6

layout(binding = IMU_HDR) uniform sampler2D tHDRAccum;
layout(binding = IMU_DIFFUSE) uniform sampler2D tDiffuse;

vec3 tonemap(vec3 c) { return c / ( c + vec3(1.0) ); }
float GammaIEC(float c) { return c <= 0.0031308 ? c * 12.92 : 1.055 * pow(c, 1/2.4) -0.055; }
vec3 GammaIEC(vec3 c)
{
    return vec3(
        GammaIEC(c.r),
        GammaIEC(c.g),
        GammaIEC(c.b));
}

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