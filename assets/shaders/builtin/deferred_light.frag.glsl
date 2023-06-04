@include "version.inc.glsl"
@include "common.inc.glsl"
@include "fragment_uniforms.inc.glsl"

out vec3 hdrColor;

in INTERFACE
{
    vec4 positionVS;
} In;

struct lightinginput_t {
    vec3 normal;
    vec4 fragPosVS;
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
layout(binding = IMU_FRAGPOS)   uniform sampler2D tFragPos;
//layout(binding = IMU_EMMISIVE)  uniform sampler2D tEmissive;
layout(binding = IMU_SHADOW)    uniform sampler2DShadow tShadow;

float ApproxPow ( float fBase, float fPower ) {
	return asfloat( uint( fPower * float( asuint( fBase ) ) - ( fPower - 1 ) * 127 * ( 1 << 23 ) ) );
}
vec3 fresnelSchlick ( vec3 f0, float costheta ) {
	//const float baked_spec_occl = saturate( 50.0 * dot( f0, vec3( 0.3333 ) ) );
	return f0 + ( 1.0 - f0 ) * ApproxPow( saturate( 1.0 - costheta ), 5.0 );
}

/*****************************************************/
/* Cook-Torrance specular BRDF. Based on DOOM 2016   */
/*****************************************************/
vec4 specBRDF( vec3 N, vec3 V, vec3 L, vec3 f0, float roughness ) {
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

// Performs shadow calculation with PCF
// Returns 1.0 if fragment is in shadow 0.0 otherwise
float ShadowCalculation(vec4 fragPosLightSpace, float NdotL)
{
    float bias = gShadowBias * (1.0 - NdotL);
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    float xOffset = gOneOverShadowRes;
    float yOffset = gOneOverShadowRes;
    float factor = 0.0;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    // float closestDepth = texture(tShadow, projCoords.xy).r; 
    // check whether current frag pos is in shadow
    // return step(closestDepth, (projCoords.z - bias));

    const float z = projCoords.z - bias;
     for (int y = -1 ; y <= 1 ; y++) {
        for (int x = -1 ; x <= 1 ; x++) {
            vec2 Offsets = vec2(x * xOffset, y * yOffset);
            vec3 UVC = vec3(projCoords.xy + Offsets, z);
            factor += texture(tShadow, UVC);
        }
    }
    return 1.0 - (factor / 9.0);
}


void main()
{
    lightinginput_t inputs;
    
    {
        vec2 texCoord           = gl_FragCoord.xy * g_freqLowFrag.screenSize.zw;
        inputs.normal           = texture( tNormal, texCoord ).xyz * 2.0 - 1.0;
        inputs.sampleAmbient    = texture( tDiffuse, texCoord );
        inputs.samplePBR        = texture( tAORM, texCoord );
        inputs.lightPos         = g_freqHighFrag.lightOrigin.xyz;
        inputs.lightColor       = g_freqHighFrag.lightColor.rgb * g_freqHighFrag.lightColor.w;
        inputs.normal           = normalize(inputs.normal);

        vec3 viewRay = vec3(In.positionVS.xy * (gFarClipDistance / In.positionVS.z), gFarClipDistance);
        float nDepth = -texture( tFragPos, texCoord ).x;
        inputs.fragPosVS = vec4(viewRay * nDepth, 1.0);
    }
    /*********************** Lighting  ****************************/
    inputs.viewDir = normalize(g_freqLowFrag.viewOrigin.xyz - inputs.fragPosVS.xyz);
    {
        vec3 L = inputs.lightPos - inputs.fragPosVS.xyz;
        float d = length(L);
        float Kr = d / gLightRange ;
        Kr *= Kr;
        Kr *= Kr;
        inputs.attenuation = max( min( 1.0 - Kr, 1 ), 0 ) / ( 1.0 + gLinearAttnFactor * d + gQuadraticAttnFactor * d*d);
        inputs.lightDir = L / d;
    }
    
    vec3 finalColor = vec3(0.0);
    {
        if (gSpotLight > 0.0)
        {
            // spotlight
            float spotAttenuation = 0.02;
            float spotDdotL = saturate(dot (-inputs.lightDir, g_freqHighFrag.spotDirection.xyz));
            if (spotDdotL >= gSpotCosCutoff)
            {
                float spotValue = smoothstep(gSpotCosCutoff, gSpotCosInnerCutoff, spotDdotL);
                spotAttenuation += ApproxPow(spotValue, gSpotExponent);
            }
            inputs.attenuation *= spotAttenuation;
        }

        vec3 f0 = mix( vec3(0.04), inputs.sampleAmbient.xyz, inputs.samplePBR.y );
        vec4 spec = specBRDF(inputs.normal, inputs.viewDir, inputs.lightDir, f0, inputs.samplePBR.x);
        vec3 F = spec.rgb;
        float Ks = spec.w;
        float NdotL = saturate( dot(inputs.normal, inputs.lightDir) );

        float shadow = 1.0;
        if (gShadowScale > 0.0)
        {
            vec4 fragPosLight = g_freqHighFrag.lightProjMatrix * inputs.fragPosVS;
            shadow = 1.0 - (gShadowScale * ShadowCalculation(fragPosLight, NdotL));
        }

        vec3 Kd = (vec3(1.0) - F) * (1.0 - inputs.samplePBR.y);

        vec3 light = NdotL * inputs.lightColor * inputs.attenuation * shadow;
        finalColor = (Kd * inputs.sampleAmbient.xyz + F * Ks) * light;
        //finalColor = max( inputs.fragPos.xyz, 0.01);
        //finalColor = vec3(1.0) - exp(-finalColor * gExposure);
    }
    /*****************************************************************/

    hdrColor = ( finalColor );
}