#define FLG_X_COVERAGE_SHIFT 0
#define FLG_X_COVERAGE_MASK 3
#define FLG_COVERAGE_SOLID 0
#define FLG_COVERAGE_MASKED 1
#define FLG_COVERAGE_BLEND 2

#define gRoughnessFactor g_freqHighFrag.matMRFactor.x
#define gMetallicFactor g_freqHighFrag.matMRFactor.y
#define gShadowBias g_freqLowFrag.shadowparams.z
#define gShadowScale g_freqLowFrag.shadowparams.y
#define gOneOverShadowRes g_freqLowFrag.shadowparams.x
#define gAlphaCutoff g_freqHighFrag.alphaCutoff.x

#define gSpotCosCutoff g_freqHighFrag.spotLightParams.x
#define gSpotCosInnerCutoff g_freqHighFrag.spotLightParams.y
#define gSpotExponent g_freqHighFrag.spotLightParams.z
#define gSpotLight g_freqHighFrag.spotLightParams.w

#define gLightRange g_freqHighFrag.lightAttenuation.x
#define gLinearAttnFactor g_freqHighFrag.lightAttenuation.y
#define gQuadraticAttnFactor g_freqHighFrag.lightAttenuation.z

#define gExposure g_freqLowFrag.params.x
#define gFlagsX g_freqHighFrag.params.x

#define gNearClipDistance g_freqLowFrag.nearFarClip.x
#define gFarClipDistance g_freqLowFrag.nearFarClip.y
#define gScreenSize g_freqLowFrag.screenSize.xy
#define gInvScreenSize g_freqLowFrag.screenSize.zw

layout(binding = UBB_FREQ_LOW_FRAG, std140) uniform uboFreqLowFrag
{
	mat4 invProjMatrix;
	vec4 debugparams;
	vec4 shadowparams;
	vec4 screenSize;
	vec4 nearFarClip;
	vec4 params;
	vec4 viewOrigin;
	vec4 ambientColor;

} g_freqLowFrag;

layout(binding = UBB_FREQ_HIGH_FRAG, std140) uniform uboFreqHighFrag
{
	mat4 lightProjMatrix;
	vec4 matDiffuseFactor;
	vec4 matMRFactor;
	vec4 matEmissiveFactor;
	vec4 alphaCutoff;
	vec4 lightOrigin;
	vec4 lightColor;
	vec4 lightAttenuation;
	vec4 spotLightParams;
	vec4 spotDirection;
	vec4 params;
} g_freqHighFrag;
