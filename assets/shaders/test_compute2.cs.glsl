#include "version.inc.glsl"

#ifndef LOCAL_SIZE_X
#define LOCAL_SIZE_X 1
#endif
#ifndef LOCAL_SIZE_Y
#define LOCAL_SIZE_Y 1
#endif

layout(local_size_x = LOCAL_SIZE_X, local_size_y = LOCAL_SIZE_Y) in;
layout(rgba8, binding = 0) uniform image2D g_tOutput;

uniform float g_fFa;
uniform float g_fAngle;

const vec3 black = vec3(0.0);

void main()
{

	ivec2 pc = ivec2(gl_GlobalInvocationID.xy);

	float y = 256.0 + 240.0 * sin(float(pc.x - 1.5 * g_fAngle)/16.0) * cos(float(pc.x + g_fAngle)/128.0);
	float x = 256.0 + 240.0 * cos(float(pc.x + g_fAngle)/64.0);
	float r = x/512.0;
	float b = y/512.0;
	float g = 0.5 * r + g_fFa * b;

	vec3 white = vec3(r,g,b);

	float H = 20.0 + 18.0 * cos(float(pc.x + g_fAngle)/256.0);

	vec4 pixel = vec4( mix( black, white,
		abs(y - pc.y) < H || abs(x - pc.x) < H), 1.0 );

	imageStore(g_tOutput, pc, pixel);
}