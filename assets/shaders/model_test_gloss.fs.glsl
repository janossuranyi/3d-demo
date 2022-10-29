#version 450 core

// Material Flags
#define MF_METALLIC_ROUGNESS 1
#define MF_SPECULAR_GLOSSINESS (1<<1)
#define MF_DIFFUSE_TEX (1<<8)
#define MF_NORMAL_TEX (1<<9)
#define MF_METALLIC_ROUGHNESS_TEX (1<<10)
#define MF_SPECULAR_GLOSSINESS_TEX (1<<11)
#define MF_EMISSIVE_TEX (1<<12)
#define Mf_OCCLUSION_TEX (1<<13)


// PBR Specular-Glossiness model

const float PI = 3.14159265359;

const float kLightRadius = 5;
vec3 kLightColor = vec3(1.0,1.0,1.0) * 20;

layout (location = 0) out vec4 FragColor;

in INTERFACE {
   vec3 FragPos;
   vec2 TexCoords;
   vec3 TangentLightPos;
   vec3 TangentViewPos;
   vec3 TangentFragPos;
   vec3 Normal;
} In;

layout(std140, binding = 2) uniform cb_camera
{
    vec4 cam_position;
    vec4 cam_target;
    vec4 cam_direction;
    vec4 cam_up;
	float znear;
	float zfar;
	float yfov;
	float ascept;
};

layout(std140, binding = 5) uniform cb_material
{
    vec4 baseColor;
    vec4 specularColor;
    vec4 emissiveColor;
    float param1;   // metalness
    float param2;   // roughness/shininess
    uint flags;         

} material;

layout(binding = 0) uniform sampler2D samp0_albedo;
layout(binding = 1) uniform sampler2D samp1_normal;
layout(binding = 2) uniform sampler2D samp2_pbr;
layout(binding = 3) uniform sampler2D samp3_emissive;
layout(binding = 4) uniform sampler2D samp4_ao;

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

float GammaIEC(float c)
{
    return c <= 0.0031308 ? c * 12.92 : 1.055 * pow(c, 1/2.4) -0.055;
}

vec3 GammaIEC(vec3 c)
{
    return vec3(
        GammaIEC(c.r),
        GammaIEC(c.g),
        GammaIEC(c.b));
}

vec3 Gamma(vec3 c)
{
    return pow(c, vec3(1.0/2.2));
}

vec3 tonemap(vec3 c)
{
    return c / ( c + vec3(1.0) );
}

float luminance(vec3 c)
{
    return dot(c, vec3(0.299, 0.587, 0.114));
}

float rgb2luma(vec3 rgb){
    return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));
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

struct light_t
{
    vec3 p;
    vec3 c;
};

light_t lights[] = light_t[](
    light_t(In.TangentLightPos, kLightColor),
    light_t(In.TangentLightPos + vec3(-10,0,0), vec3(2,2,2)),
    light_t(In.TangentLightPos + vec3(10,0,0), vec3(2,2,2))
);

void main()
{
    vec3 Cd, N;
    vec4 Csg;

    if ((material.flags & MF_DIFFUSE_TEX) != 0) {
        Cd = texture(samp0_albedo, In.TexCoords).rgb;
    } else {
        Cd = material.baseColor.rgb;
    }
    
    if ((material.flags & MF_SPECULAR_GLOSSINESS_TEX) != 0) {
        Csg = texture(samp2_pbr, In.TexCoords);
    } else {
        Csg = material.specularColor;
    }
    
    vec3 Cs = Csg.rgb;

    if ((material.flags & MF_NORMAL_TEX) != 0) {
        N = texture(samp1_normal, In.TexCoords).rgb;
        N = normalize(N * 2.0 - 1.0);  // this normal is in tangent space
    } else {
        N = normalize(In.Normal);
    }

    vec3 V = normalize(In.TangentViewPos - In.TangentFragPos);

    vec3 finalColor = vec3(0.0);

    for (int i = 0; i < lights.length(); ++i)
    {
        vec3 lightPos = lights[i].p;

        vec3 L = lightPos - In.TangentFragPos;
        float distance = length(L);
        L /= distance;

        vec3 radiance = light_radiance(distance, kLightRadius, lights[i].c, 0.005);

        specBRDF_t specular = specBRDF(N, V, L, Cs, Csg.a);

        // add to outgoing radiance Lo

        float NdotL = max(dot(N, L), 0.0);
        vec3 color = (specular.kD * Cd / PI + specular.color) * radiance * NdotL;
        finalColor += color;
    }


    //finalColor = GammaIEC(tonemap(finalColor));

    FragColor = vec4(finalColor,1);
}

