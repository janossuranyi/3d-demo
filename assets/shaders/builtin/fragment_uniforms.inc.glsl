#define FLG_X_COVERAGE_SHIFT 0
#define FLG_X_COVERAGE_MASK 3
#define FLG_COVERAGE_SOLID 0
#define FLG_COVERAGE_MASKED 1
#define FLG_COVERAGE_BLEND 2

#define gEmissiveFactor g_freqHighFrag.matEmissiveFactor.xyz
#define gEmissiveStrength g_freqHighFrag.matEmissiveFactor.w
#define gRoughnessFactor g_freqHighFrag.matMRFactor.x
#define gMetallicFactor g_freqHighFrag.matMRFactor.y
#define gAlphaCutoff g_freqHighFrag.alphaCutoff.x


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
	vec4 screenSize;
	vec4 nearFarClip;
	vec4 params;
	vec4 viewOrigin;
	vec4 ambientColor;

} g_freqLowFrag;

layout(binding = UBB_FREQ_HIGH_FRAG, std140) uniform uboFreqHighFrag
{
	vec4 matDiffuseFactor;
	vec4 matMRFactor;
	vec4 matEmissiveFactor;
	vec4 alphaCutoff;
	vec4 params;
} g_freqHighFrag;
