@include "defs.inc"
@include "fragment_uniforms.inc.glsl"
@include "common.inc.glsl"

layout(binding = IMU_HDR)       uniform sampler2D tHDRAccum;
layout(binding = IMU_DIFFUSE)   uniform sampler2D tDiffuse;
layout(binding = IMU_EMMISIVE)  uniform sampler2D tBloom;

out vec3 fragColor0;
in vec2 texcoord;

vec3 gamma(vec3 c)
{
    return pow(c, vec3(1.0/FS_ViewParams.params.y));
}

void main()
{
    vec2 texCoord = gl_FragCoord.xy * FS_ViewParams.screenSize.zw;
    vec3 color = texture(tHDRAccum, texCoord).xyz;
    vec3 bloom = texture(tBloom, texCoord).xyz;
    color += bloom * FS_ViewParams.bloomParams.y;
    color *= gExposure;

    fragColor0 = tonemap_filmic( color );
}