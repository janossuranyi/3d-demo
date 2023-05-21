@include "version.inc.glsl"
@include "common.inc.glsl"
@include "uniforms.inc.glsl"
// Line 73

//#define LIKE_A_DOOM

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
    mat3 tbn;
    vec3 normal;
    vec3 vertexNormal;
    vec4 sampleAmbient;
    vec4 samplePBR;
    vec4 spec;
    vec3 lightPos;
    vec3 lightColor;
    vec3 viewDir;
    vec3 lightDir;
    float attenuation;
};


layout(binding = IMU_DIFFUSE)   uniform sampler2D tDiffuse;
layout(binding = IMU_NORMAL)    uniform sampler2D tNormal;
layout(binding = IMU_AORM)      uniform sampler2D tAORM;
layout(binding = IMU_EMMISIVE)  uniform sampler2D tEmissive;

vec3 ReconstructNormal(vec3 normalTS) { return normalize(normalTS * 2.0 - 1.0); }
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
	//const float baked_spec_occl = saturate( 50.0 * dot( f0, vec3( 0.3333 ) ) );
	return f0 + ( 1.0 - f0 ) * ApproxPow( saturate( 1.0 - costheta ), 5.0 );
}

/*******************************************************************************/
/* Cook-Torrance specular BRDF. Based on https://learnopengl.com/PBR/Lighting   */
/*******************************************************************************/
vec4 specBRDF( vec3 N, vec3 V, vec3 L, vec3 f0, float roughness ) {
	const vec3 H = normalize( V + L );
	float m = roughness*roughness;
	m *= m;
	float NdotH = saturate( dot( N, H ) );
	float spec = (NdotH * NdotH) * (m - 1) + 1;
	spec = m / ( spec * spec + 1e-8 );
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
	float Gv = saturate( dot( N, V ) ) * (1.0 - k) + k;
	float Gl = saturate( dot( N, L ) ) * (1.0 - k) + k;
	spec /= ( 4.0 * Gv * Gl + 1e-8 );
	return vec4(fresnelSchlick( f0, dot( H, V ) ), spec);
}

/*****************************************************/
/* Cook-Torrance specular BRDF. Based on DOOM 2016   */
/*****************************************************/
vec4 specBRDF_DOOM( vec3 N, vec3 V, vec3 L, vec3 f0, float roughness ) {
	const vec3 H = normalize( V + L );
	float m = ( 0.2 + roughness * 0.8 );
	m *= m;
	m *= m;
	float m2 = m * m;
	float NdotH = saturate( dot( N, H ) );
	float spec = (NdotH * NdotH) * (m2 - 1) + 1;
	spec = m / ( spec * spec + 1e-8 );
	float Gv = saturate( dot( N, V ) ) * (1.0 - m) + m;
	float Gl = saturate( dot( N, L ) ) * (1.0 - m) + m;
	spec /= ( 4.0 * Gv * Gl + 1e-8 );
	return vec4(fresnelSchlick( f0, dot( L, H ) ), spec);
}

const vec3 CANDLE_COLOR = vec3(255, 87, 51)/255.0;

void main()
{
    lightinginput_t inputs = lightinginput_t(mat3(0),vec3(0),vec3(0),vec4(0),vec4(0),vec4(0),vec3(0),vec3(0),vec3(0),vec3(0),0);
    
    {
        vec3 localTangent       = normalize( In.tangent.xyz );
        vec3 localNormal        = normalize( In.normal );
        vec3 derivedBitangent   = normalize( cross( localNormal, localTangent ) * In.tangent.w );
        vec3 normalTS           = ReconstructNormal(texture(tNormal, In.texCoord).xyz) * vec3(1.0, -1.0, 1.0);
        inputs.tbn      = mat3(localTangent,derivedBitangent,localNormal);
        inputs.normal   = (inputs.tbn * normalTS);
        inputs.vertexNormal = localNormal;
    }

    vec4 color = vec4( 0.0 );
    int debflags = int( ubo.debugFlags.x );
    
    inputs.sampleAmbient = ubo.matDiffuseFactor * SRGBlinear( texture( tDiffuse, In.texCoord ) );
    inputs.samplePBR = texture( tAORM, In.texCoord ) * vec4(1.0, ubo.matMRFactor.x, ubo.matMRFactor.y, 1.0);

    inputs.lightPos = vec3(ubo.viewOrigin);
    inputs.lightColor = CANDLE_COLOR * 5;
    /*****************************************************************/
    inputs.viewDir = normalize(ubo.viewOrigin.xyz - In.fragPos.xyz);
    {
        vec3 L = (inputs.lightPos - In.fragPos.xyz);
        float Ld = length(L);
        L /= Ld;
        inputs.lightDir = L;
        inputs.attenuation = 1.0 / (1.0 + 0.2*Ld + 1.0*Ld*Ld);
    }
    
    vec3 final = vec3(0);
    {
        float exposure = 3;
        float NdotL = saturate( dot(inputs.normal, inputs.lightDir) );

        vec3 f0 = mix( vec3(0.04), inputs.sampleAmbient.xyz, inputs.samplePBR.b );
#ifdef LIKE_A_DOOM
        vec4 spec = specBRDF_DOOM(inputs.normal, inputs.viewDir, inputs.lightDir, f0, inputs.samplePBR.g);
#else
        vec4 spec = specBRDF(inputs.normal, inputs.viewDir, inputs.lightDir, f0, inputs.samplePBR.g);
#endif
        vec3 F = spec.rgb;
        float Ks = spec.w;

        vec3 Kd = (vec3(1.0) - F) * (1.0 - inputs.samplePBR.b);
        final = (Kd * inputs.sampleAmbient.xyz + F * Ks) * NdotL * inputs.lightColor * inputs.attenuation;
        final = vec3(1.0) - exp(-final * exposure);

        inputs.spec = vec4(F * Ks, Ks);
    }
    /*****************************************************************/

    uint flg_x = asuint(ubo.flags.x);

    if ( debflags == 0 )
    {
        uint localCoverage = (flg_x >> FLG_X_COVERAGE_SHIFT) & FLG_X_COVERAGE_MASK;
        if ( localCoverage == FLG_COVERAGE_MASKED && inputs.sampleAmbient.a < ubo.alphaCutoff.x )
        { 
            discard;
        }
        color.xyz = GammaIEC( tonemap( final )); 
    }
    else if ( debflags == 1 )
    {
        color.xyz = EncodeColor( inputs.normal );
    }
    else if ( debflags == 2 )
    {
        color.xyz = EncodeColor(inputs.vertexNormal);
    }
    else if ( debflags == 3 )
    {
        color.xyz = inputs.spec.www;
    }
    else if ( debflags == 4 )
    {
        color.xyz = inputs.spec.xyz;
    }
    else if ( debflags == 5 )
    {
        color.x = ( gl_FragCoord.z * gl_FragCoord.w );
    }
    else if ( debflags  == 6 )
    {
        color.xyz = vec3(0, 1-inputs.samplePBR.g, inputs.samplePBR.b);
    }

    fragColor = color * In.color;
}