#include "JSE.h"

JseResult JseGfxCore::Init(bool debugMode)
{
    return Init_impl(debugMode);
}

JseResult JseGfxCore::CreateSurface(const JseSurfaceCreateInfo& createSurfaceInfo)
{
    if (createSurfaceInfo.width == 0 && createSurfaceInfo.height == 0 && createSurfaceInfo.fullScreen == false) {
        return JseResult::INVALID_SURFACE_DIMENSION;
    }

    return CreateSurface_impl(createSurfaceInfo);
}

JseResult JseGfxCore::CreateBuffer(const JseBufferCreateInfo& createBufferInfo)
{
    return CreateBuffer_impl(createBufferInfo);
}

JseResult JseGfxCore::UpdateBuffer(const JseBufferUpdateInfo& bufferUpdateInfo)
{
    return UpdateBuffer_impl(bufferUpdateInfo);
}

JseResult JseGfxCore::DestroyBuffer(JseBufferID bufferId)
{
    return DestroyBuffer_impl(bufferId);
}

JseResult JseGfxCore::CreateImage(const JseImageCreateInfo& createImageInfo)
{
    return CreateImage_impl(createImageInfo);
}

JseResult JseGfxCore::CreateTexture(const JseImageCreateInfo& createImageInfo)
{
    return CreateImage_impl(createImageInfo);
}

JseResult JseGfxCore::UpdateImageData(const JseImageUploadInfo& imageUploadInfo)
{
    return UpdateImageData_impl(imageUploadInfo);
}

JseResult JseGfxCore::CreateGraphicsPipeline(const JseGraphicsPipelineCreateInfo& graphicsPipelineCreateInfo)
{
    return CreateGraphicsPipeline_impl(graphicsPipelineCreateInfo);
}

JseResult JseGfxCore::GetDeviceCapabilities(JseDeviceCapabilities& dest)
{
    return GetDeviceCapabilities_impl(dest);
}

JseResult JseGfxCore::SetVSyncInterval(int interval)
{
    return SetVSyncInterval_impl(interval);
}

void JseGfxCore::Shutdown()
{
    Shutdown_impl();
}
