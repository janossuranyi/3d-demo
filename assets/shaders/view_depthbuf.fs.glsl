#version 330 core

in vec2 vso_TexCoord;
uniform sampler2D samp0;

out vec4 FS_OUT;

float linearize_depth(float original_depth) {
    float near = 1.0;
    float far = 1700.0;
    return (2.0 * near) / (far + near - original_depth * (far - near));
}

void main() {
	float depth = linearize_depth( texture(samp0, vso_TexCoord).r );
	FS_OUT = vec4(vec3(depth,0,0), 1-depth);
}