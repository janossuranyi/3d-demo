@include "version.inc.glsl"

in INTERFACE
{
    vec4 fragPos;
    vec4 color;
    vec4 normal;
} In;

@include "uniforms.inc.glsl"

out vec4 fragColor;

void main()
{
    fragColor = In.color;
}