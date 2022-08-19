#version 330 core
#extension GL_ARB_explicit_uniform_location : enable

out vec4 fragColor;

in vec4 vo_position;
in vec4 vo_qtangent;
in vec2 vo_st;
in vec4 vo_color;

const float kGamma = 2.2;
const float kInvGamma = 1/2.2;

uniform sampler2D samp0;
uniform sampler2D samp1;
uniform sampler2D samp2;

vec4 rgb_to_srgb(vec4 p) {
	return vec4(
		pow(p.r, kGamma),
		pow(p.g, kGamma),
		pow(p.b, kGamma),
		p.a
	);
}

vec4 srgb_to_rgb(vec4 p) {
	return vec4(
		pow(p.r, kInvGamma),
		pow(p.g, kInvGamma),
		pow(p.b, kInvGamma),
		p.a
	);
}

void main() {
	
	vec4 albedo = texture( samp0, vo_st );
	vec4 metal = texture( samp1, vo_st );
	vec4 normal = texture( samp2, vo_st );
//	fragColor = rgb_to_srgb( color * vo_color );
	fragColor = mix(albedo, mix(metal, normal, 0.2), 0.2);
	
}