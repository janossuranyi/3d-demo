#define gViewMatrix VS_ViewParams.viewMatrix
#define gModelMatrix VS_DrawParams.localToWorldMatrix
#define gNormalMatrix VS_DrawParams.normalMatrix

layout(binding = UBB_VS_VIEW_PARAMS, std140) uniform uboVSViewParams
{
	mat4 viewMatrix;
	mat4 projectMatrix;
	mat4 invProjectMatrix;
} VS_ViewParams;

layout(binding = UBB_VS_DRAW_PARAMS, std140) uniform uboVSDrawParams
{
	mat4 localToWorldMatrix;
	mat4 modelViewMatrix;
	mat4 WVPMatrix;
	mat4 normalMatrix;
} VS_DrawParams;
