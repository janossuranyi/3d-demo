
in INTERFACE {
	vec4 fragColor;
} In;

layout(location = 0) out vec4 fragColor;

float GammaIEC(float c)
{
    return c <= 0.0031308 ? c * 12.92 : 1.055 * pow(c, 1/2.4) -0.055;
}

vec4 GammaIEC(vec4 c)
{
    return vec4(
        GammaIEC(c.r),
        GammaIEC(c.g),
        GammaIEC(c.b),
        c.a);
}

void main() {
	fragColor = In.fragColor;
}
