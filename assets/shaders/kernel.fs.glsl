#version 330 core

in vec2 vso_TexCoord;
out vec4 fso_Color;

uniform sampler2D samp0;
uniform float g_kernel[9];
uniform float g_offset;

float GammaIEC(float c)
{
    return c <= 0.0031308 ? c * 12.92 : 1.055 * pow(c, 1/2.4) -0.055;
}

vec3 GammaIEC(vec3 c)
{
    return vec3(
        GammaIEC(c.r),
        GammaIEC(c.g),
        GammaIEC(c.b));
}

void main() {

	vec2 offsets[9] = vec2[](
		vec2(-g_offset, g_offset), // top-left
		vec2(0.0f, g_offset), // top-center
		vec2(g_offset, g_offset), // top-right
		vec2(-g_offset, 0.0f),   // center-left
		vec2(0.0f, 0.0f),   // center-center
		vec2(g_offset, 0.0f),   // center-right
		vec2(-g_offset, -g_offset), // bottom-left
		vec2(0.0f, -g_offset), // bottom-center
		vec2(g_offset, -g_offset)  // bottom-right
	);

	vec3 sampleTex[9];
	for (int i = 0; i < 9; i++)
	{
		sampleTex[i] = vec3(texture(samp0, vso_TexCoord.st + offsets[i]));
	}

	vec3 col = vec3(0.0);
	for (int i = 0; i < 9; i++)
	{
		col += sampleTex[i] * g_kernel[i];
	}

	fso_Color = vec4(GammaIEC(col), 1.0);
}
