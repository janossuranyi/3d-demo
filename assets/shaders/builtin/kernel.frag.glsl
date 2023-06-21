@include "version.inc.glsl"
@include "defs.inc"
@include "common_uniforms.inc.glsl"

in vec2 texCoord;

out vec4 fragColor0;

uniform sampler2D tInput;
#define g_XOffset g_backendData.params[0].x
#define g_YOffset g_backendData.params[0].y
#define g_iKernel g_backendData.params[0].z

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

void main() {

	vec2 offsets[9] = vec2[](
		vec2(-g_XOffset, g_YOffset), // top-left
		vec2(0.0f, g_YOffset), // top-center
		vec2(g_XOffset, g_YOffset), // top-right
		vec2(-g_XOffset, 0.0f),   // center-left
		vec2(0.0f, 0.0f),   // center-center
		vec2(g_XOffset, 0.0f),   // center-right
		vec2(-g_XOffset, -g_YOffset), // bottom-left
		vec2(0.0f, -g_YOffset), // bottom-center
		vec2(g_XOffset, -g_YOffset)  // bottom-right
	);

	vec3 sampleTex[ 9 ];
	for (int i = 0; i < 9; i++)
	{
		sampleTex[ i ] = vec3( texture( tInput, texCoord + offsets[ i ] ) );
	}

	vec3 col = vec3( 0.0 );
    uint k = uint(g_iKernel);
	for (int i = 0; i < 9; i++)
	{
		col += sampleTex[ i ] * kernels[ k ][ i ];
	}

	fragColor0 = vec4( col, 1.0 );
}
