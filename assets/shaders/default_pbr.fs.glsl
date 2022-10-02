#version 450 core
#extension GL_ARB_explicit_uniform_location : enable

out vec4 fragColor;

in INTERFACE {
	vec3	worldPos;
	vec2	texcoord;
	vec3	normal;
	vec3	tanWorldPos;
	vec3	tanViewPos;
	vec3	tanLightPos;
	vec4	color;
} In;

const float kGamma = 2.2;
const float kInvGamma = 1/2.2;

#ifdef HAS_DIFFUSE_TEX
layout(binding = 0) uniform sampler2D tex2d_albedo;
#endif
#ifdef HAS_NORMAL_TEX
layout(binding = 1) uniform sampler2D tex2d_normal;
#endif
#ifdef HAS_METALLIC_ROUGHNESS_TEX
layout(binding = 2) uniform sampler2D tex2d_pbr;
#endif
#ifdef HAS_EMISSIVE_TEX
layout(binding = 3) uniform sampler2D tex2d_emissive;
#endif
#ifdef HAS_OCCLUSION_TEX
layout(binding = 4) uniform sampler2D tex2d_occlusion;
#endif

#include <material>

struct specBRDF_t
{
    vec3 color;
    vec3 kS;
    vec3 kD;
};

float saturate(float a) { return clamp(a, 0.0, 1.0); }


vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}  


vec3 light_radiance(float d, float r, vec3 c, float cutoff)
{
    float denom = d/r + 1;
    float attenuation = 1.0 / (denom * denom);

    attenuation = (attenuation - cutoff) / (1 - cutoff);
    attenuation = max(attenuation, 0);

    return c * attenuation;
}


vec3 DeGamma(vec3 c)
{
    return pow(c, vec3(2.2));
}

vec3 Gamma(vec3 c)
{
    return pow(c, vec3(1.0/2.2));
}

vec3 tonemap(vec3 c)
{
    return c / ( c + vec3(1.0) );
}


/* cook-torrance BRDF */
specBRDF_t specBRDF ( vec3 N, vec3 V, vec3 L, vec3 f0, float smoothness ) {

    specBRDF_t res;

	const vec3 H = normalize( V + L );
	float m = ( 1 - smoothness * 0.8 );
	m *= m;
	m *= m;
	float m2 = m * m;
	float NdotH = saturate( dot( N, H ) );
	float spec = (NdotH * NdotH) * (m2 - 1) + 1;
	spec = m2 / ( spec * spec + 1e-8 );
	float Gv = saturate( dot( N, V ) ) * (1.0 - m) + m;
	float Gl = saturate( dot( N, L ) ) * (1.0 - m) + m;
	spec /= ( 4.0 * Gv * Gl + 1e-8 );

    res.kS = fresnelSchlick( dot( L, H ), f0 );
    res.kD = vec3(1.0) - res.kS;
	res.color = res.kS * spec;

    return res;
}


void main() {
	
	vec3 Cd, N;
	vec4 Cs;
	
#ifdef HAS_DIFFUSE_TEX
	Cd = (texture(tex2d_albedo, In.texcoord) * baseColorFactor).rgb;
#else
	Cd = baseColorFactor.rgb;
#endif
#ifdef HAS_METALLIC_ROUGHNESS_TEX
	Cs = texture(tex2d_pbr, In.texcoord);
#else
#ifdef HAS_SPECULAR_GLOSSINESS_TEX
	Cs = texture(tex2d_pbr, In.texcoord);
#endif
#endif


}