@include "version.inc.glsl"

out vec4 fragColor;

layout(binding = 0) uniform sampler2D tInput;

in vec2 texCoord;
void main()
{
    fragColor = vec4(texture(tInput, texCoord).rgb,1);
}