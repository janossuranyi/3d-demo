@include "version.inc.glsl"
@include "common.inc.glsl"
@include "fragment_uniforms.inc.glsl"

out vec3 hdrColor;

struct lightinginput_t {
    vec3 normal;
    vec3 fragPos;
    vec4 sampleAmbient;
    vec4 samplePBR;
    vec4 spec;
    vec3 lightPos;
    vec3 lightColor;
    vec3 viewDir;
    vec3 lightDir;
    vec3 ambient;
    float attenuation;
};

layout(binding = IMU_DIFFUSE)   uniform sampler2D tDiffuse;
layout(binding = IMU_NORMAL)    uniform sampler2D tNormal;
layout(binding = IMU_AORM)      uniform sampler2D tAORM;
layout(binding = IMU_FRAGPOS)   uniform sampler2D tFragPos;
//layout(binding = IMU_EMMISIVE)  uniform sampler2D tEmissive;
//layout(binding = IMU_SHADOW)    uniform sampler2DShadow tShadow;

vec3 ReconstructNormal(vec3 normalTS) { return normalize(normalTS * 2.0 - 1.0); }

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

void main()
{
    lightinginput_t inputs;
    vec2 texCoord = gl_FragCoord.xy / g_freqLowFrag.screenSize.xy;
    {
        vec3 N = vec3( texture(tNormal, texCoord).xy, 0.5 ) * 2.0 - 1.0;
        N.z = abs(sqrt(1.0 - inputs.normal.x*inputs.normal.x - inputs.normal.y*inputs.normal.y));
        inputs.normal = normalize(N);
    }
    
    inputs.sampleAmbient = texture( tDiffuse, texCoord );
    inputs.samplePBR = texture( tAORM, texCoord );
    inputs.lightPos = vec3(g_freqLowFrag.lightOrig);
    inputs.lightColor = g_freqLowFrag.lightColor.rgb * g_freqLowFrag.lightColor.w;
    inputs.ambient = g_freqLowFrag.ambientColor.rgb * g_freqLowFrag.ambientColor.w * inputs.sampleAmbient.xyz;
    inputs.fragPos = texture(tFragPos, texCoord).xyz;

    /*********************** Lighting  ****************************/
    inputs.viewDir = normalize(g_freqLowFrag.viewOrigin.xyz - inputs.fragPos);
    {
        vec3 L = inputs.lightPos - inputs.fragPos;
        float d = length(L);
        inputs.attenuation = 1.0 / ( 1.0 + gLinearAttnFactor * d + gQuadraticAttnFactor * d * d );
        inputs.lightDir = L / d;
    }
    
    vec3 finalColor = vec3(0.0);
    {
        if (gSpotLight > 0.0)
        {
            // spotlight
            float spotAttenuation = 0.02;
            float spotDdotL = saturate(dot (-inputs.lightDir, g_freqLowFrag.spotDirection.xyz));
            if (spotDdotL >= gSpotCosCutoff)
            {
                float spotValue = smoothstep(gSpotCosCutoff, gSpotCosInnerCutoff, spotDdotL);
                spotAttenuation += ApproxPow(spotValue, gSpotExponent);
            }
            inputs.attenuation *= spotAttenuation;
        }

        vec3 f0 = mix( vec3(0.04), inputs.sampleAmbient.xyz, inputs.samplePBR.b );
        vec4 spec = specBRDF(inputs.normal, inputs.viewDir, inputs.lightDir, f0, inputs.samplePBR.g);
        vec3 F = spec.rgb;
        float Ks = spec.w;
        float NdotL = saturate( dot(inputs.normal, inputs.lightDir) );
        //float shadow = 1.0-(gShadowScale*ShadowCalculation(In.fragPosLight, NdotL));

        vec3 Kd = (vec3(1.0) - F) * (1.0 - inputs.samplePBR.b);

        vec3 light = NdotL * inputs.lightColor * inputs.attenuation;
        finalColor = (Kd * inputs.sampleAmbient.xyz + F * Ks) * light;
        finalColor += inputs.ambient;
        //inputs.spec = vec4(F * Ks, Ks);
    }
    /*****************************************************************/

    hdrColor = finalColor;
}