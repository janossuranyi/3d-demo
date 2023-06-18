layout(binding = UBB_COMMON_DATA) uniform uboCommonData
{
    vec4 renderTargetRes;
    vec4 shadowRes;
    vec4 bloomRes;
    vec4 ssaoKernel[64];
} g_commonData;

layout(binding = UBB_BACKEND_DATA) uniform uboBackendData
{
    vec4 params[16];
} g_backendData;
