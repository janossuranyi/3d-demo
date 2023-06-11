
#define gShadowBias g_lightData.shadowparams.z
#define gShadowScale g_lightData.shadowparams.y
#define gOneOverShadowRes g_lightData.shadowparams.x

#define gSpotCosCutoff g_lightData.spotLightParams.x
#define gSpotCosInnerCutoff g_lightData.spotLightParams.y
#define gSpotExponent g_lightData.spotLightParams.z
#define gSpotLight g_lightData.spotLightParams.w

#define gLightRange g_lightData.lightAttenuation.x
#define gLinearAttnFactor g_lightData.lightAttenuation.y
#define gQuadraticAttnFactor g_lightData.lightAttenuation.z


layout(binding = UBB_LIGHT_DATA, std140) uniform uboLightData
{
    mat4 lightProjMatrix;
    vec4 shadowparams;
	vec4 lightOrigin;
	vec4 lightColor;
	vec4 lightAttenuation;
	vec4 spotLightParams;
	vec4 spotDirection;
} g_lightData;
