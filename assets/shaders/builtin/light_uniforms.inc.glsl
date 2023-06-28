
#define gShadowBias g_lightData.shadowparams.z
#define gShadowScale g_lightData.shadowparams.y
#define gOneOverShadowRes g_lightData.shadowparams.x

#define gSpotCosCutoff g_lightData.params.x
#define gSpotCosInnerCutoff g_lightData.params.y
#define gSpotExponent g_lightData.params.z
#define gSpotLight g_lightData.params.w

#define gLightRange g_lightData.attenuation.x
#define gLinearAttnFactor g_lightData.attenuation.y
#define gQuadraticAttnFactor g_lightData.attenuation.z


layout(binding = UBB_LIGHT_DATA, std140) uniform uboLightData
{
    mat4 projectMatrix;
    vec4 shadowparams;
	vec4 origin;
	vec4 direction;
	vec4 color;
	vec4 attenuation;
	vec4 params;
} g_lightData;
