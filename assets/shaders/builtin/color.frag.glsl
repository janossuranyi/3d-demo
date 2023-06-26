
out vec4 fragColor;

in INTERFACE
{
    vec4 color;
} In;

void main()
{
    fragColor = In.color;
}