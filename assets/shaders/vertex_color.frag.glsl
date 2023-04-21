#include "version.inc.glsl"

in INTERFACE
{
    vec4 fragPos;
    vec4 color;
    vec4 normal;
} In;

layout(binding = 0, std140) uniform uboUniforms
{
    mat4 localToWorldMatrix;
    mat4 worldToViewMatrix;
    mat4 worldToviewProjectionMatrix;
    vec4 viewOrigin;
};

out vec4 fragColor;

void main()
{
    fragColor = In.color;
}