@include "defs.inc"

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

#define gSpotCosCutoff g_freqLowFrag.spotLightParams.x
#define gSpotCosInnerCutoff g_freqLowFrag.spotLightParams.y
#define gSpotExponent g_freqLowFrag.spotLightParams.z
#define gSpotLight g_freqLowFrag.spotLightParams.w

#define gConstantAttnFactor g_freqLowFrag.lightAttenuation.x
#define gLinearAttnFactor g_freqLowFrag.lightAttenuation.y
#define gQuadraticAttnFactor g_freqLowFrag.lightAttenuation.z

#define gExposure g_freqLowFrag.params.x
#define gFlagsX g_freqHighFrag.params.x

layout(binding = UBB_FREQ_LOW_FRAG, std140) uniform uboFreqLowFrag
{
	vec4 debugparams;
	vec4 shadowparams;
	vec4 screenSize;
	vec4 nearFarClip;
	vec4 params;
	vec4 viewOrigin;
	vec4 lightOrig;
	vec4 lightColor;
	vec4 lightAttenuation;
	vec4 spotLightParams;
	vec4 spotDirection;
	vec4 ambientColor;

} g_freqLowFrag;

layout(binding = UBB_FREQ_HIGH_FRAG, std140) uniform uboFreqHighFrag
{
	mat4 lightProjMatrix;
	vec4 matDiffuseFactor;
	vec4 matMRFactor;
	vec4 matEmissiveFactor;
	vec4 alphaCutoff;
	vec4 lightOrig;
	vec4 lightColor;
	vec4 lightAttenuation;
	vec4 spotLightParams;
	vec4 spotDirection;
	vec4 params;
} g_freqHighFrag;
