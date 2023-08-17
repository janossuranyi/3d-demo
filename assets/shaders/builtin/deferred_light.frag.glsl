@include "common.inc.glsl"
@include "defs.inc"
@include "fragment_uniforms.inc.glsl"
@include "light_uniforms.inc.glsl"
@include "common_uniforms.inc.glsl"

layout(depth_unchanged) out float gl_FragDepth;

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
#   ifdef LIGHT_DIR
layout(binding = IMU_DEFAULT)   uniform sampler2D tAO;
#   endif

float ApproxPow ( float fBase, float fPower ) {
	return asfloat( uint( fPower * float( asuint( fBase ) ) - ( fPower - 1 ) * 127 * ( 1 << 23 ) ) );
}
vec3 fresnelSchlick ( vec3 f0, float costheta ) {
	//const float baked_spec_occl = saturate( 50.0 * dot( f0, vec3( 0.3333 ) ) );
	return f0 + ( 1.0 - f0 ) * ApproxPow( saturate( 1.0 - costheta ), 5.0 );
}

/********************************/
/* Cook-Torrance specular BRDF. */
/********************************/
vec4 specBRDF( vec3 N, vec3 V, vec3 L, vec3 f0, float roughness ) {
	const vec3 H = normalize( V + L );
	float m = ( 0.2 + roughness * 0.8 );
	m *= m;
	//m *= m;
	float m2 = m * m;
	float NdotH = saturate( dot( N, H ) );
	float spec = (NdotH * NdotH) * (m2 - 1) + 1;
	spec = m2 / ( spec * spec + 1e-8 );
	float Gv = saturate( dot( N, V ) ) * (1.0 - m) + m;
	float Gl = saturate( dot( N, L ) ) * (1.0 - m) + m;
	spec /= ( 4.0 * Gv * Gl + 1e-8 );
    vec4 res = vec4( fresnelSchlick( f0, dot( L, H ) ), spec );

	return res;
}

vec4 specBRDF_j(vec3 N, vec3 V, vec3 L, vec3 F0, float r)
{
    const vec3 H = normalize(V + L);
    const float HdotV  = max(dot(H, V), 0.0);
    const float NdotH  = max(dot(N, H), 0.0);
    const float NdotV  = max(dot(N, V), 0.0);
    const float NdotL  = max(dot(N, L), 0.0);

    // DistributionGGX
    float a = r;
    a *= a;
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

    return vec4(F, spec);
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
    const float xOffset = gOneOverShadowRes;
    const float yOffset = gOneOverShadowRes;
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
        vec2 texCoord           = gl_FragCoord.xy * FS_ViewParams.screenSize.zw;
        inputs.normal           = NormalOctDecode( texture( tNormal, texCoord ).xy, false );
        inputs.sampleAmbient    = texture( tDiffuse, texCoord );
        inputs.samplePBR        = texture( tAORM, texCoord );
        inputs.lightColor       = g_lightData.color.rgb * g_lightData.color.w;
#       ifdef LIGHT_DIR
        inputs.occlusion        = texture(tAO, texCoord).x;
        inputs.occlusion        = g_sharedData.params[0].x == 1.0 ? inputs.occlusion : 1.0;
        inputs.lightDir         = g_lightData.origin.xyz;
        inputs.fragPosVS        = reconstructPositionVS( In.positionVS.xyz, texCoord );
#       else
        vec3 viewRay            = vec3( In.positionVS.xy * (gFarClipDistance / In.positionVS.z), gFarClipDistance );
        float nDepth            = -texture( tFragPosZ, texCoord ).x;
        inputs.fragPosVS        = vec4( viewRay * nDepth, 1.0 );
        inputs.lightPos         = g_lightData.origin.xyz;
#       endif

    }
    /*********************** Lighting  ****************************/
    inputs.viewDir = normalize(/*FS_ViewParams.viewOrigin.xyz*/ - inputs.fragPosVS.xyz);
    
#   ifdef LIGHT_SPOT_POINT
    {
        vec3 L = inputs.lightPos - inputs.fragPosVS.xyz;
        float d = length(L);
        float Kr = d / gLightRange ;
        Kr *= Kr;
        Kr *= Kr;
        inputs.attenuation = max( min( 1.0 - Kr, 1.0 ), 0.0 ) / ( 1.0 + gLinearAttnFactor * d + gQuadraticAttnFactor * d*d);
        inputs.lightDir = L / d;
    }
#   endif
    {
#   ifdef LIGHT_SPOT_POINT
        if (gSpotLight > 0.0)
        {
            // spotlight
            float spotAttenuation = 0.0;
            float spotDdotL = saturate(dot (-inputs.lightDir, g_lightData.direction.xyz));
            if (spotDdotL >= gSpotCosCutoff)
            {
                float spotValue = smoothstep(gSpotCosCutoff, gSpotCosInnerCutoff, spotDdotL);
                spotAttenuation += ApproxPow(spotValue, gSpotExponent);
            }
            inputs.attenuation *= spotAttenuation;
        }
#   endif
        vec3 f0 = mix( vec3(0.04), inputs.sampleAmbient.xyz, inputs.samplePBR.y );
        vec4 spec = specBRDF(inputs.normal, inputs.viewDir, inputs.lightDir, f0, inputs.samplePBR.x /* roughness */);

        const vec3  F = spec.rgb;
        const float Ks = spec.w;
        const vec3  Kd = (vec3(1.0) - F) * (1.0 - inputs.samplePBR.y);
        const float NdotL = saturate( dot(inputs.normal, inputs.lightDir) );

        float shadow = 1.0;

        if (gShadowScale > 0.0)
        {
            vec4 fragPosLight = g_lightData.projectMatrix * inputs.fragPosVS;
            shadow = 1.0 - (gShadowScale * ShadowCalc(fragPosLight, NdotL));
            // inputs.lightColor *= texture(lightMap, 1.0-coords).rgb;
        }

#   ifdef LIGHT_SPOT_POINT
        vec3 light =
            inputs.lightColor 
            * inputs.attenuation 
            * NdotL 
            * shadow;

        finalColor = light * (Kd * inputs.sampleAmbient.xyz + F * Ks);
#   else
        vec3 ambient =
            inputs.sampleAmbient.xyz
            * FS_ViewParams.ambientColor.xyz
            * FS_ViewParams.ambientColor.w;

        vec3 light =
            inputs.lightColor 
            * NdotL 
            * shadow;

        finalColor = ambient * inputs.occlusion + (light * (Kd * inputs.sampleAmbient.xyz + F * Ks));
#   endif
        //finalColor = vec3(Ks) * inputs.lightColor * inputs.attenuation * NdotL * shadow;;
    }
    /*****************************************************************/
    fragColor0 = finalColor;
}