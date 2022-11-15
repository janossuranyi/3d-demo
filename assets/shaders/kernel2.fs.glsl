#include "version.inc.glsl"

in INTERFACE {
	vec2 uv;
} In;

out vec4 FragColor;

uniform sampler2D g_tInput;
uniform float g_fOffset;
uniform int g_iKernel;

#define KERNEL_BLUR 0
#define KERNEL_BOTTOM_SOBEL 1
#define KERNEL_IDENTITY 2
#define KERNEL_EMBOSS 3
#define KERNEL_LEFT_SOBEL 4
#define KERNEL_OUTLINE 5
#define KERNEL_RIGHT_SOBEL 6
#define KERNEL_SHARPEN 7
#define KERNEL_TOP_SOBEL 8

const float kernels[9][9] = {
	{
		0.0625f, 0.125f, 0.0625f,
		0.125f, 0.25f, 0.125f,
		0.0625f, 0.125f, 0.0625f
	},
	{
		-1.0f, -2.0f, -1.0f,
		 0.0f,  0.0f,  0.0f,
		 1.0f,  2.0f,  1.0f
	},
	{
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
	},
	{
		-2.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 1.0f,
		 0.0f,  1.0f, 2.0f
	},
	{
		1.0f, 0.0f, -1.0f,
		2.0f, 0.0f, -2.0f,
		1.0f, 0.0f, -1.0f
	},
	{
		-1.0f, -1.0f, -1.0f,
		-1.0f,  8.0f, -1.0f,
		-1.0f, -1.0f, -1.0f
	},
	{
		-1.0f, 0.0f, 1.0f,
		-2.0f, 0.0f, 2.0f,
		-1.0f, 0.0f, 1.0f
	},
	{
		 0.0f, -1.0f,  0.0f,
		-1.0f,  5.0f, -1.0f,
		 0.0f, -1.0f,  0.0f
	},
	{
		 1.0f,  2.0f,  1.0f,
		 0.0f,  0.0f,  0.0f,
		-1.0f, -2.0f, -1.0f
	}
};

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
		vec2(-g_fOffset, g_fOffset), // top-left
		vec2(0.0f, g_fOffset), // top-center
		vec2(g_fOffset, g_fOffset), // top-right
		vec2(-g_fOffset, 0.0f),   // center-left
		vec2(0.0f, 0.0f),   // center-center
		vec2(g_fOffset, 0.0f),   // center-right
		vec2(-g_fOffset, -g_fOffset), // bottom-left
		vec2(0.0f, -g_fOffset), // bottom-center
		vec2(g_fOffset, -g_fOffset)  // bottom-right
	);

	vec3 sampleTex[9];
	for (int i = 0; i < 9; i++)
	{
		sampleTex[i] = vec3(texture(g_tInput, In.uv + offsets[i]));
	}

	vec3 col = vec3(0.0);
	for (int i = 0; i < 9; i++)
	{
		col += sampleTex[i] * kernels[g_iKernel][i];
	}

	FragColor = vec4(GammaIEC(col), 1.0);
}
