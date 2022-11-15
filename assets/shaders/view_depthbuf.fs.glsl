#include "version.inc.glsl"

in INTERFACE {
	vec2 uv;
} In;

uniform sampler2D samp0;
uniform float g_fFarPlane;

out vec4 FS_OUT;

float linearize_depth(float original_depth) {
	float z = original_depth*2-1.0;
    float near = 1.0;
    float far = g_fFarPlane;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main() {
	float depth = linearize_depth( texture(samp0, In.uv).r ) / g_fFarPlane;
	FS_OUT = vec4(vec3(depth), 1.0);
}