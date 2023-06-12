@include "version.inc.glsl"

out vec4 FragColor;

layout(binding = 0) uniform sampler2D g_tInput;

const int M = 8;

uniform vec2 g_vDirection;
in vec2 texCoord;

void main()
{ 
	{
		vec2 invSize = 1.0 / textureSize(g_tInput,0);
		vec2 step = invSize * g_vDirection;
		float w[ M ] = { 0.027062858, 0.088897429, 0.18941596, 0.26192293, 0.23510250, 0.13697305, 0.051778302, 0.0088469516 };
		float b_off[ M ] = { -6.3269038, -4.3775406, -2.4309988, -0.48611468, 1.4584296, 3.4039848, 5.3518057, 7.00000000 };
		vec3 sum = vec3( 0.0, 0.0, 0.0 );
		for ( int s = 0; s < M; s++ ) {
			sum += texture( g_tInput, ( texCoord + step * b_off[ s ]) ).rgb * w[ s ];
		}
		FragColor = vec4(sum,1);
	};
}