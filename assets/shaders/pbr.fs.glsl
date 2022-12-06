#include "version.inc.glsl"

const float PI = 3.14159265359;

const float kLightRadius = 5;

float saturate(float a) { return clamp(a, 0.0, 1.0); }
vec3 saturate(vec3 a) { return clamp(a, 0.0, 1.0); }
float dot2 ( vec2 a, vec2 b ) { return dot( a, b ); }
float asfloat ( uint x ) { return uintBitsToFloat( x ); }
float asfloat ( int x ) { return intBitsToFloat( x ); }
uint asuint ( float x ) { return floatBitsToUint( x ); }
int asint ( float x ) { return floatBitsToInt( x ); }
vec2 asfloat ( uvec2 x ) { return uintBitsToFloat( x ); }
vec2 asfloat ( ivec2 x ) { return intBitsToFloat( x ); }
uvec2 asuint ( vec2 x ) { return floatBitsToUint( x ); }
ivec2 asint ( vec2 x ) { return floatBitsToInt( x ); }
vec3 asfloat ( uvec3 x ) { return uintBitsToFloat( x ); }
vec3 asfloat ( ivec3 x ) { return intBitsToFloat( x ); }
uvec3 asuint ( vec3 x ) { return floatBitsToUint( x ); }
ivec3 asint ( vec3 x ) { return floatBitsToInt( x ); }
vec4 asfloat ( uvec4 x ) { return uintBitsToFloat( x ); }
vec4 asfloat ( ivec4 x ) { return intBitsToFloat( x ); }
uvec4 asuint ( vec4 x ) { return floatBitsToUint( x ); }
ivec4 asint ( vec4 x ) { return floatBitsToInt( x ); }
float fmax3 ( float f1, float f2, float f3 ) { return max( f1, max( f2, f3 ) ); }
float fmin3 ( float f1, float f2, float f3 ) { return min( f1, min( f2, f3 ) ); }

float SRGBlinear ( float value ) { if ( value <= 0.04045 ) {
		return ( value / 12.92 );
	} else {
		return pow( ( value / 1.055 ) + 0.0521327, 2.4 );
	}
}
vec3 SRGBlinear ( vec3 sRGB ) {
	vec3 outLinear;
	outLinear.r = SRGBlinear( sRGB.r );
	outLinear.g = SRGBlinear( sRGB.g );
	outLinear.b = SRGBlinear( sRGB.b );
	return outLinear;
}
vec4 SRGBlinear ( vec4 sRGBA ) {
	vec4 outLinear = vec4( SRGBlinear( sRGBA.rgb ), 1 );
	outLinear.a = SRGBlinear( sRGBA.a );
	return outLinear;
}
float DeGamma ( float value ) {
	return SRGBlinear( value );
}
vec4 DeGamma ( vec4 color ) {
	return SRGBlinear( color );
}
vec3 DeGamma ( vec3 color ) {
	return SRGBlinear( color );
}

float light_attenuation(float d, float r, float cutoff)
{
    float denom = d/r + 1;
    float attenuation = 1.0 / (denom * denom);

    attenuation = (attenuation - cutoff) / (1 - cutoff);
    attenuation = max(attenuation, 0);

    return attenuation;
}

vec3 tonemap(vec3 c)
{
    return c / ( c + vec3(1.0) );
}

float GetLuma ( vec3 c ) {
	return dot( c, vec3( 0.2126, 0.7152, 0.0722 ) );
}

float ApproxPow ( float fBase, float fPower ) {
	return asfloat( uint( fPower * float( asuint( fBase ) ) - ( fPower - 1 ) * 127 * ( 1 << 23 ) ) );
}


vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}  


vec3 fresnelSchlick2( float costheta, vec3 f0 ) {
	const float baked_spec_occl = saturate( 50.0 * dot( f0, vec3( 0.3333 ) ) );
	return saturate( f0 + ( baked_spec_occl - f0 ) * ApproxPow( saturate( 1.0 - costheta ), 5.0 ) );
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

vec3 specBRDF_doom ( vec3 N, vec3 V, vec3 L, vec3 f0, float roughness, out vec3 F ) {
	const vec3 H = normalize( V + L );
	float m = roughness;
	m *= m;
	m *= m;
	float m2 = m * m;
	float NdotH = saturate( dot( N, H ) );
	float spec = (NdotH * NdotH) * (m2 - 1) + 1;
	spec = m2 / ( spec * spec + 1e-8 );
	float Gv = saturate( dot( N, V ) ) * (1.0 - m) + m;
	float Gl = saturate( dot( N, L ) ) * (1.0 - m) + m;
	spec /= ( 4.0 * Gv * Gl + 1e-8 );
    F = fresnelSchlick( dot( L, H ), f0 );
	return F * spec;
}

vec3 specBRDF(vec3 N, vec3 V, vec3 L, vec3 F0, float r, out vec3 Fout)
{
    vec3 H = normalize(V + L);
    float HdotV  = max(dot(H, V), 0.0);
    float NdotH  = max(dot(N, H), 0.0);
    float NdotV  = max(dot(N, V), 0.0);
    float NdotL  = max(dot(N, L), 0.0);

    // DistributionGGX
    float a = r;
    a *= a;
    a *= a;
    float denom = (NdotH * NdotH) * (a - 1.0) + 1.0;
    float NDF = a / (PI * denom * denom);

    // GeometrySmith
    a = (r + 1.0);
    float k   = (r*r) / 8.0;
    float Gv  = NdotV / (NdotV * (1.0 - k) + k);
    float Gl  = NdotL / (NdotL * (1.0 - k) + k);
    float spec = (NDF * Gv * Gl) / (4.0 * NdotL * NdotL + 0.0001);

    vec3 F = fresnelSchlick(HdotV, F0);

    Fout = F;
    return F * spec;
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

struct light_t {
    vec3 pos;
    vec3 color;    
};

uniform sampler2D samp_basecolor;
uniform sampler2D samp_normal;
uniform sampler2D samp_pbr;


in INTERFACE {
   vec3 FragPos;
   vec2 TexCoords;
   vec3 TangentViewPos;
   vec3 TangentFragPos;
   vec3 TangentNormal;
   vec3 TangentLightPos;
   vec4 Color;
   vec3 tangent;
} In;

out vec4 FragColor;

uniform light_t g_lights[1];

const float kContrastFactor = 259.0/255.0;

float contrastFactor(float C)
{
    C = min(max(C, -1.0), 1.0);
    return (kContrastFactor * (C + 1.0)) / (kContrastFactor - C);
}

float contrast(float c, float f)
{
    return saturate((c - 0.5) * contrastFactor(f) + 0.5);
}

vec3 contrast(vec3 c, float f)
{
    return saturate((c - 0.5) * contrastFactor(f) + 0.5);
}
void main()
{

    vec3 Cd, N;
    vec4 Cs;

    Cd = texture(samp_basecolor, In.TexCoords).rgb;
    Cs = texture(samp_pbr, In.TexCoords);

    N = texture(samp_normal, In.TexCoords).rgb;
    //N = GammaIEC(N);
    N.y = 1.0 - N.y;
    N = normalize(N * 2.0 - 1.0);  // this normal is in tangent space

    vec3 V = normalize(In.TangentViewPos - In.TangentFragPos);

    vec3 finalColor = vec3(0.0);

    vec3 ambient = vec3(0.03) * Cd;

    float roughness = Cs.g;
    float metalness = Cs.b;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, Cd, metalness);

    float clip_max = 1.0/255.0;

    for(int i = 0; i < g_lights.length(); ++i)
    {
        light_t light = g_lights[i];

        vec3 lightPos = In.TangentLightPos;

        vec3 L = lightPos - In.TangentFragPos;
        
        float distance = length(L);
        L /= distance;

        float attenuation = light_attenuation(distance, 1.2, clip_max);
        if (attenuation == 0.0) continue;

        vec3 F ;

        float NdotL       = max( dot( N, L ), 0.0 );
        vec3 spec         = specBRDF(N,V,L,F0,roughness,F);

        vec3 kD = vec3(1.0) - F;
        kD *= 1.0 - metalness;

        finalColor += (kD * Cd / PI + spec) * attenuation * light.color * NdotL;
    }

    vec3 color = tonemap(finalColor + ambient);

    color = GammaIEC(color);

    FragColor = vec4(color, 1.0);
}

