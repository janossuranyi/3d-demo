@include "version.inc.glsl"

#define	IMU_DIFFUSE 0
#define	IMU_NORMAL 1
#define IMU_AORM 2
#define IMU_EMMISIVE 3
#define IMU_DEPTH 4
#define IMU_FRAGPOS 5
#define IMU_HDR 6
#define IMU_DEFAULT 7

const float PI = 3.14159265359;
const float ONE_OVER_PI = 1.0/3.14159265359;

float asfloat ( uint x ) { return uintBitsToFloat( x ); }
uint asuint ( float x ) { return floatBitsToUint( x ); }

float saturate(float x) { return clamp(x, 0.0, 1.0); }
vec2 saturate(vec2 x) { return clamp(x, 0.0, 1.0); }
vec3 saturate(vec3 x) { return clamp(x, 0.0, 1.0); }
vec4 saturate(vec4 x) { return clamp(x, 0.0, 1.0); }

vec3 tonemap(vec3 c) { return c / ( c + vec3(1.0) ); }
float GammaIEC(float c) { return c <= 0.0031308 ? c * 12.92 : 1.055 * pow(c, 1/2.4) -0.055; }
vec3 GammaIEC(vec3 c)
{
    return vec3(
        GammaIEC(c.r),
        GammaIEC(c.g),
        GammaIEC(c.b));
}
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

vec4 SRGBlinear ( vec4 sRGBA ) { return vec4( SRGBlinear( sRGBA.rgb ), sRGBA.a );}

#define SHADER_UNIFORMS_BINDING 0

layout(binding = SHADER_UNIFORMS_BINDING, std140) uniform uboUniforms
{
    mat4 localToWorldMatrix;
    mat4 worldToViewMatrix;
	mat4 projectionMatrix;
    mat4 WVPMatrix;
	mat4 normalMatrix;
    vec4 viewOrigin;
	vec4 matDiffuseFactor;
	vec4 matMRFactor;
	vec4 alphaCutoff;
	vec4 debugFlags;
	vec4 clipPlanes;
	vec4 user01;
	vec4 user02;
	vec4 user03;
	vec4 user04;
	vec4 user05;
	vec4 user06;
	vec4 user07;
	float clipNear;
	float clipFar;
} ubo;


in INTERFACE
{
    vec4 fragPos;
    vec2 texCoord;
    vec4 color;
    vec4 tangent;
    vec3 normal;
} In;


out vec4 fragColor;

struct lightinginput_t {
    vec3 normal;
    vec3 vertexNormal;
    vec4 sampleAmbient;
    vec4 samplePBR;
    mat3 tbn;
};

layout(binding = IMU_DIFFUSE)   uniform sampler2D tDiffuse;
layout(binding = IMU_NORMAL)    uniform sampler2D tNormal;
layout(binding = IMU_AORM)      uniform sampler2D tAORM;
layout(binding = IMU_EMMISIVE)  uniform sampler2D tEmissive;

vec3 ReconstructNormal(vec3 normalTS) { return (normalTS * 2.0 - 1.0); }
vec3 EncodeColor(vec3 N) { return saturate( (1.0 + N) * 0.5 ); }

float linearize_depth(float original_depth) {
	float z = original_depth * 2.0 - 1.0;
    float near = ubo.clipPlanes.x;
    float far  = ubo.clipPlanes.y;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

float ApproxPow ( float fBase, float fPower ) {
	return asfloat( uint( fPower * float( asuint( fBase ) ) - ( fPower - 1 ) * 127 * ( 1 << 23 ) ) );
}

vec3 fresnelSchlick ( vec3 f0, float costheta ) {
	const float baked_spec_occl = saturate( 50.0 * dot( f0, vec3( 0.3333 ) ) );
	return saturate( f0 + ( baked_spec_occl - f0 ) * ApproxPow( saturate( 1.0 - costheta ), 5.0 ) );
}

vec3 specBRDF ( vec3 N, vec3 V, vec3 L, vec3 f0, float smoothness ) {
	const vec3 H = normalize( V + L );
	float m = ( 1 - smoothness * 0.8 );
	//m *= m;
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

vec3 specBRDF_sj(vec3 N, vec3 V, vec3 L, vec3 F0, float smoothness)
{
    const vec3 H = normalize(V + L);
    const float HdotV  = max(dot(H, V), 0.0);
    const float NdotH  = max(dot(N, H), 0.0);
    const float NdotV  = max(dot(N, V), 0.0);
    const float NdotL  = max(dot(N, L), 0.0);

    // DistributionGGX
    float r = 1 - smoothness * 0.8;
    float a = r*r;
    a *= a;
    float denom = (NdotH * NdotH) * (a - 1.0) + 1.0;
    float NDF = a / (PI * denom * denom);

    // GeometrySmith
    a = (r + 1.0);
    float k   = (a*a) / 8.0;
    float Gv  = NdotV / (NdotV * (1.0 - k) + k);
    float Gl  = NdotL / (NdotL * (1.0 - k) + k);
    float spec = (NDF * Gv * Gl) / (4.0 * NdotL * NdotL + 1e-8);

    vec3 F = fresnelSchlick(F0, HdotV);

    return F * spec;
}

void main()
{
    
    lightinginput_t inputs = lightinginput_t(vec3(0),vec3(0),vec4(0),vec4(0),mat3(0));

    {
        vec3 localTangent       = normalize( In.tangent.xyz );
        vec3 localNormal        = normalize( In.normal );
        vec3 derivedBitangent   = normalize( cross( localNormal, localTangent ) * In.tangent.w );
        vec3 normalTS           = vec3(1,-1,1) * (texture(tNormal, In.texCoord).xyz * 2.0 - 1.0);
        inputs.tbn      = mat3(localTangent,derivedBitangent,localNormal);
        inputs.normal   = inputs.tbn * normalTS;
        inputs.vertexNormal = localNormal;
    }

    vec4 color = vec4( 0.0 );
    int debflags = int( ubo.debugFlags.x );
    
    inputs.sampleAmbient = ubo.matDiffuseFactor * SRGBlinear( texture( tDiffuse, In.texCoord ) );
    inputs.samplePBR = texture( tAORM, In.texCoord );
    inputs.samplePBR.g = 1.0 - inputs.samplePBR.g;

    vec3 lightPos = vec3(ubo.viewOrigin);
    vec3 lightColor = vec3(4.5, 1.5, 1.0) ;
    float specFactor = 1.0;

    if ( debflags == 0 )
    {
        vec3 V = normalize(ubo.viewOrigin.xyz - In.fragPos.xyz);
        vec3 L = (lightPos - In.fragPos.xyz);
        float Ld = length(L);
        L /= Ld;
        
        float attenuation = 1.0 / (1.0 + 0.5*Ld + 1*Ld*Ld);

        float NdotL = max( dot( inputs.normal, L ), 0.0 );
        
        vec3 f0 = mix(vec3(0.04), inputs.sampleAmbient.xyz, inputs.samplePBR.b);
        vec3 spec = specBRDF_sj(inputs.normal, V, L, f0, inputs.samplePBR.g);
        vec3 diffuseFactor = (1.0 - spec) * (1.0 - inputs.samplePBR.b);
        vec3 final = (diffuseFactor * inputs.sampleAmbient.xyz + spec) * NdotL * lightColor * attenuation;
        color.xyz = GammaIEC( tonemap( final ));
    }
    else if ( ( debflags & 1 ) == 1 )
    {
        color.xyz = EncodeColor( inputs.normal );
    }
    else if ( ( debflags & 2 ) == 2 )
    {
        color.xyz = inputs.samplePBR.xyz;
    }
    else if ( ( debflags & 4 ) == 4 )
    {
        color.xy = In.texCoord.xy;
    }
    else if ( ( debflags & 8 ) == 8 )
    {
        color.xyz = EncodeColor(In.fragPos.xyz);
    }
    else if ( ( debflags & 16 ) == 16 )
    {
        color.xyz = saturate( vec3( gl_FragCoord.z * gl_FragCoord.w ) );
    }
    else if ( ( debflags & 32 ) == 32 )
    {
        color.xyz = EncodeColor(inputs.vertexNormal);
    }

    fragColor = color * In.color;
}