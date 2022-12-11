#include "version.inc.glsl"

const float PI = 3.14159265359;
const float inv_PI = 1.0 / PI;

float saturate(float a) { return clamp(a, 0.0, 1.0); }
vec3 saturate(vec3 a) { return clamp(a, 0.0, 1.0); }
vec4 saturate(vec4 a) { return clamp(a, 0.0, 1.0); }

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

float light_radiance(float d, float r, float cutoff)
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

vec3 fresnelSchlick(vec3 F0, float cosTheta) {
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}  

/*
vec3 fresnelSchlick ( vec3 f0, float costheta ) {
	const float baked_spec_occl = saturate( 50.0 * dot( f0, vec3( 0.3333 ) ) );
	return saturate( f0 + ( baked_spec_occl - f0 ) * ApproxPow( saturate( 1.0 - costheta ), 5.0 ) );
}
*/

vec3 ReconstructNormal( vec3 normalTS, bool isFrontFacing ) {
	vec3 N = normalize(normalTS.xyz * 2.0 - 1.0);
    
	N.z = sqrt( saturate( 1 - N.x * N.x - N.y * N.y ) );
    
	if ( isFrontFacing == false ) {
		N.z = -N.z;
	}
	return N;
}
vec3 TransformNormal( vec3 normal, mat3x3 mat ) {
	//return normalize( vec3( dot( normal.xyz, mat[0].xyz), dot( normal.xyz, mat[1].xyz), dot( normal.xyz, mat[2].xyz) ) );
    return normalize(normal * mat);
}
vec3 GetWorldSpaceNormal ( vec3 normalTS, mat3x3 invTS, bool isFrontFacing ) {
	const vec3 N = ReconstructNormal( normalTS.xyz, isFrontFacing );
	return TransformNormal( N, invTS );
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
    float denom = NdotV * (1.0 - k) + k;
	
    return NdotV / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec4 specBRDF(vec3 N, vec3 V, vec3 L, vec3 F0, float roughness)
{
	const vec3 H = normalize( V + L );
    // cook-torrance brdf

	float m = roughness;
	m *= m;
	float m2 = m * m;
	float NdotH = saturate( dot( N, H ) );

    // DistributionGGX
	float spec = (NdotH * NdotH) * (m2 - 1) + 1;
	spec = m2 / ( PI * spec * spec + 1e-8 );  // <-- NDF

    // GeometrySmith
    // float r = 1.0 + roughness;
    // float k = (r*r) / 8.0;
    // GeometrySchlickGGX
	float Gv = saturate( dot( N, V ) ) * (1.0 - m) + m;
	float Gl = saturate( dot( N, L ) ) * (1.0 - m) + m;
	spec /= ( 4.0 * Gv * Gl + 1e-8 );

    vec3 F = fresnelSchlick( F0, dot( H, V ) );
    return vec4(F, spec);
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
    vec4 pos;
    vec4 color;    
};

struct lightingInput_t
{
    vec3 albedo;
    vec3 specular;
    float roughness;
    float metalness;
    float occlusion;
    vec3 normal;
    vec3 normalTS;
    vec3 view;
    vec3 position;
    vec2 texCoord;
    vec4 fragCoord;
    vec3 out_color;
    mat3 invTS;
};

uniform sampler2D samp_basecolor;
uniform sampler2D samp_normal;
uniform sampler2D samp_pbr;
uniform vec3    g_vLightOffset;
uniform int     g_iNumlights;
uniform float   g_fLightPower;

layout(std140, binding = 0) uniform LightInfoBlock
{   
    light_t g_lights[256];
};

in INTERFACE {
    vec4 normal;
    vec4 tangent;
    vec4 position;
    vec4 texcoord;
    vec4 color;
} In;

out vec4 FragColor;

void main()
{
    lightingInput_t inputs = lightingInput_t( vec3(0),vec3(0),0,0,0,vec3(0),vec3(0),vec3(0),vec3(0),vec2(0),vec4(0),vec3(0),mat3(1.0));

    inputs.texCoord = In.texcoord.xy;
    inputs.fragCoord = gl_FragCoord;
    inputs.position = In.position.xyz;

    {
        vec3 inNormal = normalize(In.normal.xyz);
        vec3 inTangent = normalize(In.tangent.xyz);
        vec3 derivedBitangent = normalize( cross( inNormal, inTangent ) * In.tangent.w );
        inputs.invTS = transpose(mat3(inTangent,derivedBitangent,inNormal));        
    }
    inputs.view = normalize( -inputs.position );

    {
        vec3 normal = texture(samp_normal, inputs.texCoord).xyz;
        normal.y = 1.0 - normal.y;
        inputs.normal = normal;        
    }

    inputs.albedo = texture(samp_basecolor, inputs.texCoord).rgb;
    {
        vec4 inMR = texture(samp_pbr, inputs.texCoord);
        inputs.roughness = inMR.g;
        inputs.metalness = inMR.b;
        inputs.occlusion = inMR.a;
    }

    bool isFrontFacing = true;
    inputs.normalTS = ReconstructNormal( inputs.normal.xyz, isFrontFacing );
    inputs.normal = GetWorldSpaceNormal( inputs.normal, inputs.invTS, isFrontFacing );

    vec3 ambient = vec3(0.001) * inputs.albedo * inputs.occlusion;
    vec4 tmp;

    inputs.out_color = vec3(0.0);

    for(int lightIdx = 0; lightIdx < g_iNumlights; ++lightIdx)
    {
        light_t light = g_lights[lightIdx];

        float NdotL;
        float clip_min = 1.0 / 255.0;
        vec3 light_position = light.pos.xyz + g_vLightOffset;
        vec3 light_vector = normalize( light_position - inputs.position );
        NdotL = saturate( dot( inputs.normal, light_vector ) );
        if (NdotL < 0) continue;
        
        float light_attenuation = 0.0;
        {
            float d = distance(light_position, inputs.position) + 0.00001;
            light_attenuation = light_radiance(d, 2, clip_min);
            if (light_attenuation < clip_min / 256.0) continue;
        }
        vec3 light_color = max(light.color.xyz * g_fLightPower, 0.0) * light_attenuation;
        vec3 light_color_final = light_color;
        {
            vec3 F0 = mix( vec3(0.04), inputs.albedo, inputs.metalness );
            /*====================================================================================*/
            vec4 spec = specBRDF( inputs.normal, inputs.view, light_vector, F0, inputs.roughness );
            /*====================================================================================*/
            vec3 Kd = vec3(1.0) - spec.xyz;
            Kd *= 1.0 - inputs.metalness;
            inputs.out_color += (Kd * inputs.albedo * inv_PI + spec.xyz * spec.w) * light_color_final * NdotL;
        }
    }

    vec3 color = tonemap( inputs.out_color + ambient );
    color = GammaIEC( color ) ;

    FragColor = vec4(color, 1.0);
}