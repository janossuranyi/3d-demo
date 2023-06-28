#define FLG_X_COVERAGE_SHIFT 0
#define FLG_X_COVERAGE_MASK 3
#define FLG_X_SSAO_SHIFT 2
#define FLG_X_SSAO_MASK 1
#define FLG_COVERAGE_SOLID 0
#define FLG_COVERAGE_MASKED 1
#define FLG_COVERAGE_BLEND 2

#define gEmissiveFactor FS_DrawParams.matEmissiveFactor.xyz
#define gEmissiveStrength FS_DrawParams.matEmissiveFactor.w
#define gRoughnessFactor FS_DrawParams.matMRFactor.x
#define gMetallicFactor FS_DrawParams.matMRFactor.y
#define gAlphaCutoff FS_DrawParams.alphaCutoff.x


#define gExposure FS_ViewParams.params.x
#define gFlagsX FS_DrawParams.params.x

#define gNearClipDistance FS_ViewParams.nearFarClip.x
#define gFarClipDistance FS_ViewParams.nearFarClip.y
#define gScreenSize FS_ViewParams.screenSize.xy
#define gInvScreenSize FS_ViewParams.screenSize.zw
#define gBloomParams FS_ViewParams.bloomParams
#define gBloomParams2 FS_ViewParams.bloomParams2

layout(binding = UBB_FS_VIEW_PARAMS, std140) uniform uboFSViewParams
{
	mat4 invProjMatrix;
	mat4 projectMatrix;
	vec4 debugparams;
	vec4 screenSize;
	vec4 nearFarClip;
	vec4 params;
	vec4 viewOrigin;
	vec4 ambientColor;
	vec4 bloomParams;
	vec4 bloomParams2;
} FS_ViewParams;

layout(binding = UBB_FS_DRAW_PARAMS, std140) uniform uboFSDrawParams
{
	vec4 matDiffuseFactor;
	vec4 matMRFactor;
	vec4 matEmissiveFactor;
	vec4 alphaCutoff;
	vec4 params;
} FS_DrawParams;
