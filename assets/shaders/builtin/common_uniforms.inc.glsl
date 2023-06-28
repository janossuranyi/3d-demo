layout(binding = UBB_COMMON_DATA) uniform uboCommonData
{
    vec4 renderTargetRes;
    vec4 shadowRes;
    vec4 bloomRes;
    vec4 ssaoKernel[ SSAO_KERNEL_SAMPLES ];
} g_commonData;

layout(binding = UBB_SHARED_DATA) uniform uboSharedData
{
    vec4 params[8];
} g_sharedData;
