#include "version.inc.glsl"

float saturate(float a) { return clamp(a, 0.0, 1.0); }
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
vec3 specBRDF ( vec3 N, vec3 V, vec3 L, vec3 f0, float roughness, out vec3 F ) {
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
    F = fresnelSchlick( f0, dot( L, H ) );
	return F * spec;
}

struct light_t {
    vec3 pos;
    vec3 color;    
};

struct lightingInput_t
{
    vec3 albedo;
    vec3 specular;
    float roughness;
    float metalness;
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
uniform vec4 g_vViewPosition;

uniform light_t g_lights[1];

in INTERFACE {
    vec4 normal;
    vec4 tangent;
    vec4 position;
    vec4 texcoord;
    vec4 viewpos;
    vec4 color;
} In;

out vec4 FragColor;

const float PI = 3.14159265359;

void main()
{
    lightingInput_t inputs = lightingInput_t( vec3(0),vec3(0),0,0,vec3(0),vec3(0),vec3(0),vec3(0),vec2(0),vec4(0),vec3(0),mat3(1.0));

    inputs.texCoord = In.texcoord.xy;
    inputs.fragCoord = gl_FragCoord;
    inputs.position = In.position.xyz;
    {
        vec3 inNormal = normalize(In.normal.xyz);
        vec3 inTangent = normalize(In.tangent.xyz);
        vec3 derivedBitangent = normalize( cross( inNormal, inTangent ) * In.tangent.w );
        inputs.invTS = transpose(mat3(inTangent,derivedBitangent,inNormal));        
    }
    vec3 view = inputs.position - In.viewpos.xyz;
    inputs.view = normalize( -view );

    vec3 normal = ( texture(samp_normal, inputs.texCoord).rgb );
    normal.y = 1.0-normal.y;
    inputs.normal = normalize(normal * 2.0 - 1.0);
    inputs.normalTS = inputs.invTS * inputs.normal;
    inputs.albedo = ( texture(samp_basecolor, inputs.texCoord).rgb );
    {
        vec4 inMR = ( texture(samp_pbr, inputs.texCoord) );
        inputs.roughness = inMR.g;
        inputs.metalness = inMR.b;        
    }

    vec3 ambient = vec3(0.001);

    for(int lightIdx = 0; lightIdx < g_lights.length(); ++lightIdx)
    {
        light_t light = g_lights[lightIdx];

        float clip_min = 1.0 / 255.0;
        vec3 light_position = inputs.invTS * light.pos;
        {
            vec3 light_vector = inputs.invTS * normalize( light_position - inputs.position );
            float NdotL = saturate( dot( inputs.normal, light_vector ) );
            if (NdotL < clip_min) continue;
        }
        float light_attenuation;
        {
            float d = distance(light_position, inputs.position) + 0.00001;
            float denom = d/5.0 + 1;
            light_attenuation = 1.0 / (denom * denom);
            if (light_attenuation < clip_min / 256.0) continue;
        }
        vec3 light_color = max(light.color,0.0) * light_attenuation;
        vec3 light_color_final = light_color;
        {
            vec3 light_vector = normalize( light_position - inputs.position);
            float NdotL = saturate( dot( inputs.normal, light_vector ) );
            vec3 F0 = mix(vec3(0.04), inputs.albedo, inputs.metalness);
            vec3 F = vec3(0);
            vec3 spec = specBRDF(inputs.normal, inputs.view, light_vector, F0, inputs.roughness, F);
            vec3 Kd = vec3(1.0) - F;
            Kd *= 1.0 - inputs.metalness;
            inputs.out_color += (Kd * inputs.albedo / PI + spec) * light_color_final * NdotL;
        }
    }
    vec3 color = tonemap(inputs.out_color + ambient);
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color.xyz, 1.0);
}