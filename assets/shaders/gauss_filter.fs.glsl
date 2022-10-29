#version 450 core
out vec4 FragColor;


in INTERFACE {
    vec2 uv;
} In;

layout(binding = 0) uniform sampler2D image;

const int M = 5;

uniform vec2 u_direction;
uniform float weight[M] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
 /*
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
	0.012318109844189502);
    */

void main()
{             
    vec2 tex_offset = 1.0 / textureSize(image, 0); // gets size of single texel
    vec3 result = texture(image, In.uv).rgb * weight[0]; // current fragment's contribution
    for(int i = 1; i < M; ++i)
    {
        vec2 offset = u_direction * vec2(tex_offset * i);
        result += texture(image, In.uv + offset).rgb * weight[i];
        result += texture(image, In.uv - offset).rgb * weight[i];
    }
    FragColor = vec4(result, 1.0);
}