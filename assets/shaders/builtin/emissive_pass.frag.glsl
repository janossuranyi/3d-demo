@include "version.inc.glsl"
@include "defs.inc"
@include "fragment_uniforms.inc.glsl"

in INTERFACE
{
    vec2 texCoord;
} In;

layout(binding = IMU_EMMISIVE)  uniform sampler2D tEmissive;

out vec4 fragColor;

void main()
{
    vec3 color = texture(tEmissive, In.texCoord).xyz;
    fragColor = vec4(color * gEmissiveFactor * gEmissiveStrength, 1.0);
}