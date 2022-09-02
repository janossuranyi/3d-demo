#version 450 core

// PBR Specular-Glossiness model

const float PI = 3.14159265359;
const float dielectricSpecular = 0.04;
const float epsilon = 0.0001;
const float kLightRadius = 5;
const vec3 kLightColor = vec3(1.0,1.0,1.0) * 15;

out vec4 FragColor;

in VS_OUT {
   vec3 FragPos;
   vec2 TexCoords;
   vec3 TangentLightPos;
   vec3 TangentViewPos;
   vec3 TangentFragPos;
} fs_in;

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

uniform sampler2D samp0_albedo;
uniform sampler2D samp1_normal;
uniform sampler2D samp2_pbr;
uniform sampler2D samp3_emissive;
uniform sampler2D samp4_ao;

/* DOOM
vec3 specBRDF ( vec3 N, vec3 V, vec3 L, vec3 f0, float smoothness ) {
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
	return fresnelSchlick( f0, dot( L, H ) ) * spec;
}
*/

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}


vec3 light_radiance(float aDistance, float aRadius, vec3 aColor)
{
    float vD2 = aDistance * aDistance;
    float vR2 = aRadius * aRadius;
    float attenuation = 1.0 / (1.0 + 2 * aDistance / aRadius + vD2 / vR2);

    return aColor * attenuation;
}


vec3 DeGamma(vec3 sRGB)
{
    return pow(sRGB, vec3(2.2));
}

vec3 Gamma(vec3 RGB)
{
    return pow(RGB, vec3(1/2.2));
}

float luminance(vec3 rgb)
{
    return (rgb.r + rgb.r + rgb.b + rgb.g + rgb.g + rgb.g) / 6;
}

struct specBRDF
{
    vec3 color;
    vec3 kS;
    vec3 kD;
};

specBRDF cook_torrance_specular(vec3 N, vec3 V, vec3 L, float roughness, vec3 F0)
{
    specBRDF res;

    // cook-torrance brdf
    vec3 H = normalize(V + L);

    float NDF = DistributionGGX(N, H, roughness);        
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
    
    vec3 numerator    = NDF * G * F;
    float NdotL       = max(dot(N, L), 0.0);                
    float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
    
    res.color = numerator / denominator;
    res.kS = F;
    res.kD = vec3(1.0) - F;

    return res;
}

void main()
{

    vec3 Cd = texture(samp0_albedo, fs_in.TexCoords).rgb;
    vec4 Csg = texture(samp2_pbr, fs_in.TexCoords);
    vec3 Cs = Csg.rgb;
    vec3 N = texture(samp1_normal, fs_in.TexCoords).rgb;
    N = normalize(N * 2.0 - 1.0);  // this normal is in tangent space


    vec3 V = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 L = fs_in.TangentLightPos - fs_in.TangentFragPos;
    float distance = length(L);
    L /= distance;

    vec3 radiance = light_radiance(distance, kLightRadius, kLightColor);

    float roughness = 1-Csg.a;
    specBRDF specular = cook_torrance_specular(N, V, L, roughness, Cs);
    
    // add to outgoing radiance Lo

    float NdotL = max(dot(N, L), 0.0);                
    vec3 color = (specular.kD * Cd / PI + specular.color) * radiance * NdotL;
    color = color / (color + vec3(1.0));
    color = Gamma(color);

    FragColor = vec4(color,1);

}

