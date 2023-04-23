
#define SHADER_UNIFORMS_BINDING 0

layout(binding = SHADER_UNIFORMS_BINDING, std140) uniform uboUniforms
{
    mat4 localToWorldMatrix;
    mat4 worldToViewMatrix;
	mat4 projectionMatrix;
    mat4 worldToviewProjectionMatrix;
    vec4 viewOrigin;
	vec4 user01;
	vec4 user02;
	vec4 user03;
	vec4 user04;
	vec4 user05;
	vec4 user06;
	vec4 user07;
} ubo;
