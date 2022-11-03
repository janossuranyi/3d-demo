#include "version.inc.glsl"

in INTERFACE {
	vec2 TexCoord;
} In;

uniform sampler2D samp0;
uniform float g_far;

out vec4 FS_OUT;

float linearize_depth(float original_depth) {
	float z = original_depth*2-1.0;
    float near = 1.0;
    float far = g_far;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main() {
	float depth = linearize_depth( texture(samp0, In.TexCoord).r ) / g_far;
	FS_OUT = vec4(vec3(depth), 1.0);
}