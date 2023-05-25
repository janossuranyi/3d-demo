@include "version.inc.glsl"
@include "common.inc.glsl"
@include "uniforms.inc.glsl"
// Line 1+51+17 = 78

#define LIKE_A_DOOM

in INTERFACE
{
    vec4 fragPos;
    vec4 fragPosLight;
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
layout(binding = IMU_SHADOW)    uniform sampler2D tShadow;

vec3 ReconstructNormal(vec3 normalTS) { return normalize(normalTS * 2.0 - 1.0); }
vec3 EncodeColor(vec3 N) { return saturate( (1.0 + N) * 0.5 ); }
vec3 Gamma(vec3 c) { return pow(c, vec3(1.0/2.2)); }

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
	spec = m2 / ( spec * spec + 1e-8 );
	float Gv = saturate( dot( N, V ) ) * (1.0 - m) + m;
	float Gl = saturate( dot( N, L ) ) * (1.0 - m) + m;
	spec /= ( 4.0 * Gv * Gl + 1e-8 );
	return vec4(fresnelSchlick( f0, dot( L, H ) ), spec);
}

const vec3 CANDLE_COLOR = vec3(255, 87, 51)/255.0;

float ShadowCalculation(vec4 fragPosLightSpace, float NdotL)
{
    float bias = 0.005 * (1.0 - NdotL);
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(tShadow, projCoords.xy).r; 
    // check whether current frag pos is in shadow
    return step(closestDepth, (projCoords.z - bias));
}

void main()
{
    lightinginput_t inputs;// = lightinginput_t(mat3(0),vec3(0),vec3(0),vec4(0),vec4(0),vec4(0),vec3(0),vec3(0),vec3(0),vec3(0),0);
    
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
    
    inputs.sampleAmbient = ubo.matDiffuseFactor * SRGBlinear( texture( tDiffuse, In.texCoord ) );
    inputs.samplePBR = texture( tAORM, In.texCoord ) * vec4(1.0, ubo.matMRFactor.x, ubo.matMRFactor.y, 1.0);

    inputs.lightPos = vec3(ubo.lightOrig);
    inputs.lightColor = ubo.lightColor.rgb * ubo.lightColor.w;
    /*****************************************************************/
    inputs.viewDir = normalize(ubo.viewOrigin.xyz - In.fragPos.xyz);
    {
        vec3 L = inputs.lightPos - In.fragPos.xyz;
        float d = length(L);
        float Kc = ubo.lightAttenuation.x;
        float Kl = ubo.lightAttenuation.y;
        float Kq = ubo.lightAttenuation.z;
        inputs.attenuation = 1.0 / (Kc + Kl*d + Kq*d*d);
        inputs.lightDir = L / d;
    }
    
    vec3 final = vec3(0);
    {
        float exposure = ubo.params.y;
        if (ubo.spotLightParams.w > 0.0)
        {
            // spotlight
            float spotAttenuation = 0.02;
            float spotDdotL = saturate(dot (-inputs.lightDir, ubo.spotDirection.xyz));
            if (spotDdotL >= ubo.spotLightParams.x)
            {
                float spotValue = smoothstep(ubo.spotLightParams.x, ubo.spotLightParams.y, spotDdotL);
                spotAttenuation += ApproxPow(spotValue, ubo.spotLightParams.z);
            }
            inputs.attenuation *= spotAttenuation;
        }

        vec3 f0 = mix( vec3(0.04), inputs.sampleAmbient.xyz, inputs.samplePBR.b );
#ifdef LIKE_A_DOOM
        vec4 spec = specBRDF_DOOM(inputs.normal, inputs.viewDir, inputs.lightDir, f0, inputs.samplePBR.g);
#else
        vec4 spec = specBRDF(inputs.normal, inputs.viewDir, inputs.lightDir, f0, inputs.samplePBR.g);
#endif
        vec3 F = spec.rgb;
        float Ks = spec.w;
        float NdotL = saturate( dot(inputs.normal, inputs.lightDir) );
        float shadow = 1.0-(0.8 * ShadowCalculation(In.fragPosLight, NdotL));

        vec3 Kd = (vec3(1.0) - F) * (1.0 - inputs.samplePBR.b);
        final = (Kd * inputs.sampleAmbient.xyz + F * Ks) * NdotL * inputs.lightColor * inputs.attenuation * shadow;
        final = vec3(1.0) - exp(-final * exposure);

        inputs.spec = vec4(F * Ks, Ks);
    }
    /*****************************************************************/

    uint params_x = asuint(ubo.params.x);
    uint debflags = uint( ubo.debugFlags.x );

    if ( debflags == 0 )
    {
        uint localCoverage = (params_x >> FLG_X_COVERAGE_SHIFT) & FLG_X_COVERAGE_MASK;
        if ( localCoverage == FLG_COVERAGE_MASKED && inputs.sampleAmbient.a < ubo.alphaCutoff.x )
        { 
            discard;
        }
        color.xyz = Gamma( tonemap( final )); 
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
        color.xyz = vec3( inputs.spec.w );
    }
    else if ( debflags == 4 )
    {
        vec3 c = inputs.spec.xyz;
        color.xyz = c;
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