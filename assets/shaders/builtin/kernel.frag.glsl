@include "defs.inc"
@include "common_uniforms.inc.glsl"

in vec2 texCoord;

out vec4 fragColor0;

uniform sampler2D tInput;
#define g_Offset g_sharedData.params[0].x
#define g_Kernel g_sharedData.params[0].y

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

	int offset = int(g_Offset);
	
	const ivec2 offsets[9] = ivec2[](
		ivec2(-offset,	offset), // top-left
		ivec2( 0,		offset), // top-center
		ivec2( offset,	offset), // top-right
		ivec2(-offset,	0), // center-left
		ivec2( 0,		0), // center-center
		ivec2( offset,	0), // center-right
		ivec2(-offset,	-offset), // bottom-left
		ivec2( 0,		-offset), // bottom-center
		ivec2( offset,	-offset)  // bottom-right
	);

	vec3 sampleTex[ 9 ];
	for (int i = 0; i < 9; i++) {
		sampleTex[ i ] = textureLodOffset( tInput, texCoord, 0.0, offsets[ i ] ).xyz;
	}

	vec3 col = vec3( 0.0 );
    uint k = uint(g_Kernel);
	k = (k >= 0 && k <= KERNEL_TOP_SOBEL) ? k : KERNEL_IDENTITY;

	for (int i = 0; i < 9; i++) {
		col += sampleTex[ i ] * kernels[ k ][ i ];
	}

	fragColor0 = vec4( col, 1.0 );
}
