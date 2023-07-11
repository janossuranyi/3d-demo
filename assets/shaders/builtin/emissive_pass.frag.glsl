@include "defs.inc"
@include "common.inc.glsl"
@include "fragment_uniforms.inc.glsl"

in INTERFACE
{
    vec2 texCoord;
} In;

layout(binding = IMU_EMISSIVE)  uniform sampler2D tEmissive;

out vec4 fragColor;

void main()
{
    vec3 color = SRGBlinear( texture(tEmissive, In.texCoord).xyz );
    fragColor = vec4(color * gEmissiveFactor * gEmissiveStrength, 1.0);
}