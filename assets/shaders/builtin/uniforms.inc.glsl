#define SHADER_UNIFORMS_BINDING 4

#define gShadowBias ubo.shadowparams.z
#define gShadowScale ubo.shadowparams.y
#define gOneOverShadowRes ubo.shadowparams.x
#define gExposure ubo.params.y
#define gFlagsX ubo.params.x
#define gSpotCosCutoff ubo.spotLightParams.x
#define gSpotCosInnerCutoff ubo.spotLightParams.y
#define gSpotExponent ubo.spotLightParams.z
#define gSpotLight ubo.spotLightParams.w
#define gRoughnessFactor ubo.matMRFactor.x
#define gMetallicFactor ubo.matMRFactor.y
#define gConstantAttnFactor ubo.lightAttenuation.x
#define gLinearAttnFactor ubo.lightAttenuation.y
#define gQuadraticAttnFactor ubo.lightAttenuation.z

layout(binding = SHADER_UNIFORMS_BINDING, std140) uniform uboUniforms
{
    mat4 localToWorldMatrix;
    mat4 WVPMatrix;
	mat4 normalMatrix;
	mat4 lightProjMatrix;
    vec4 viewOrigin;
	vec4 matDiffuseFactor;
	vec4 matMRFactor;
	vec4 alphaCutoff;
	vec4 debugFlags;
	vec4 clipPlanes;
	vec4 params;
	vec4 shadowparams;
	vec4 lightOrig;
	vec4 lightColor;
	vec4 lightAttenuation;
	vec4 spotLightParams;
	vec4 spotDirection;
} ubo;

