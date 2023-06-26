@include "common.inc.glsl"
@include "defs.inc"
@include "fragment_uniforms.inc.glsl"
@include "light_uniforms.inc.glsl"
@include "common_uniforms.inc.glsl"

out vec3 fragColor0;

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
    float occlusion;
};

layout(binding = IMU_DIFFUSE)   uniform sampler2D tDiffuse;
layout(binding = IMU_NORMAL)    uniform sampler2D tNormal;
layout(binding = IMU_AORM)      uniform sampler2D tAORM;
layout(binding = IMU_FRAGPOS)   uniform sampler2D tFragPosZ;
layout(binding = IMU_SHADOW)    uniform sampler2DShadow tShadow;
layout(binding = IMU_DEFAULT)   uniform sampler2D tAO;

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

// Performs shadow calculation with PCF
// Returns 1.0 if fragment is in shadow 0.0 otherwise
float ShadowCalc(vec4 fragPosLightSpace, float NdotL)
{
    float bias = gShadowBias * (1.0 - NdotL);
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if (abs(projCoords.x) >= 1.0 || abs(projCoords.y) >= 1.0 || abs(projCoords.z) >= 1.0)
    {
        return 0.0;
    }
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    float xOffset = gOneOverShadowRes;
    float yOffset = gOneOverShadowRes;
    float factor = 0.0;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    // check whether current frag pos is in shadow
    // return 1.0 - texture(tShadow, vec3(projCoords.xy, (projCoords.z - bias)));


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

vec4 reconstructPositionVS(vec3 viewRay, vec2 uv)
{
    float linearZ = textureLod( tFragPosZ, uv, 0 ).x * gFarClipDistance;
    return vec4( viewRay * linearZ, 1.0 );
}

void main()
{
    vec3 finalColor = vec3(0.0);
    lightinginput_t inputs;    
    {
        vec2 texCoord           = gl_FragCoord.xy * g_freqLowFrag.screenSize.zw;
        inputs.normal           = NormalOctDecode( texture( tNormal, texCoord ).xy, false );
        inputs.sampleAmbient    = texture( tDiffuse, texCoord );
        inputs.samplePBR        = texture( tAORM, texCoord );
        inputs.occlusion        = texture(tAO, texCoord).x;
        inputs.occlusion        = g_backendData.params[0].x == 1.0 ? inputs.occlusion : 1.0;
        inputs.lightPos         = g_lightData.lightOrigin.xyz;
        inputs.lightColor       = g_lightData.lightColor.rgb * g_lightData.lightColor.w;

        vec3 viewRay = vec3(In.positionVS.xy * (gFarClipDistance / In.positionVS.z), gFarClipDistance);
        float nDepth = -1.0 * texture( tFragPosZ, texCoord ).x;
        inputs.fragPosVS        = vec4(viewRay * nDepth, 1.0);
        //inputs.fragPosVS        = reconstructPositionVS( In.positionVS.xyz, texCoord );
    }
    /*********************** Lighting  ****************************/
    inputs.viewDir = normalize(/*g_freqLowFrag.viewOrigin.xyz*/ - inputs.fragPosVS.xyz);
    
    {
        vec3 L = inputs.lightPos - inputs.fragPosVS.xyz;
        float d = length(L);
        float Kr = d / gLightRange ;
        Kr *= Kr;
        Kr *= Kr;
        inputs.attenuation = max( min( 1.0 - Kr, 1.0 ), 0.0 ) / ( 1.0 + gLinearAttnFactor * d + gQuadraticAttnFactor * d*d);
        inputs.lightDir = L / d;
    }
    {
        if (gSpotLight > 0.0)
        {
            // spotlight
            float spotAttenuation = 0.0;
            float spotDdotL = saturate(dot (-inputs.lightDir, g_lightData.spotDirection.xyz));
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
        vec3 Kd = (vec3(1.0) - F) * (1.0 - inputs.samplePBR.y);
        float NdotL = saturate( dot(inputs.normal, inputs.lightDir) );

        float shadow = 1.0;

        if (gShadowScale > 0.0)
        {
            vec4 fragPosLight = g_lightData.lightProjMatrix * inputs.fragPosVS;
            shadow = 1.0 - (gShadowScale * ShadowCalc(fragPosLight, NdotL));
            // inputs.lightColor *= texture(lightMap, 1.0-coords).rgb;
        }
        vec3 light =
            inputs.lightColor 
            * inputs.attenuation 
            * NdotL 
            * shadow;
            
        finalColor = light * (Kd * inputs.sampleAmbient.xyz + F * Ks);
        //finalColor = vec3(Ks) * inputs.lightColor * inputs.attenuation * NdotL * shadow;;
    }
    /*****************************************************************/

    fragColor0 = finalColor; //mix(vec3(1.0), finalColor, 0.998);
}