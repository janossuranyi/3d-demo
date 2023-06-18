#define gViewMatrix g_freqLowVert.viewMatrix
#define gModelMatrix g_freqHighVert.localToWorldMatrix
#define gNormalMatrix g_freqHighVert.normalMatrix

layout(binding = UBB_FREQ_LOW_VERT, std140) uniform uboFreqLowVert
{
	mat4 viewMatrix;
	mat4 projectMatrix;
	mat4 invProjectMatrix;
} g_freqLowVert;

layout(binding = UBB_FREQ_HIGH_VERT, std140) uniform uboFreqHighVert
{
	mat4 localToWorldMatrix;
	mat4 modelViewMatrix;
	mat4 WVPMatrix;
	mat4 normalMatrix;
} g_freqHighVert;
