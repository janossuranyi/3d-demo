#version 430

layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba16f, binding = 0) uniform restrict readonly image2D u_input_image;
layout(rgba16f, binding = 1) uniform restrict writeonly image2D u_output_image;

uniform ivec2 u_direction;

const int M = 16;
const int N = 2 * M + 1;

// sigma = 10
const float coeffs[N] = float[N](
	0.012318109844189502,
	0.014381474814203989,
	0.016623532195728208,
	0.019024086115486723,
	0.02155484948872149,
	0.02417948052890078,
	0.02685404941667096,
	0.0295279624870386,
	0.03214534135442581,
	0.03464682117793548,
	0.0369716985390341,
	0.039060328279673276,
	0.040856643282313365,
	0.04231065439216247,
	0.043380781642569775,
	0.044035873841196206,
	0.04425662519949865,
	0.044035873841196206,
	0.043380781642569775,
	0.04231065439216247,
	0.040856643282313365,
	0.039060328279673276,
	0.0369716985390341,
	0.03464682117793548,
	0.03214534135442581,
	0.0295279624870386,
	0.02685404941667096,
	0.02417948052890078,
	0.02155484948872149,
	0.019024086115486723,
	0.016623532195728208,
	0.014381474814203989,
	0.012318109844189502
);
void main()
{
	ivec2 size = imageSize(u_input_image);
	ivec2 pixel_coord = ivec2(gl_GlobalInvocationID.xy);
	if (pixel_coord.x < size.x && pixel_coord.y < size.y)
	{
		vec4 sum = vec4(0.0);
		for (int i = 0; i < N; ++i)
		{
			ivec2 pc = pixel_coord + u_direction * (i - M);
			if (pc.x < 0) pc.x = 0;
			if (pc.y < 0) pc.y = 0;
			if (pc.x >= size.x) pc.x = size.x - 1;
			if (pc.y >= size.y) pc.y = size.y - 1;
			sum += coeffs[i] * imageLoad(u_input_image, pc);
		}
		imageStore(u_output_image, pixel_coord, sum);
	}
}