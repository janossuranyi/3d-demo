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

JseResult JseGfxCore::DeleteImage(JseImageID imageId)
{
    return DeleteImage_impl(imageId);
}

JseResult JseGfxCore::CreateGraphicsPipeline(const JseGraphicsPipelineCreateInfo& graphicsPipelineCreateInfo)
{
    return CreateGraphicsPipeline_impl(graphicsPipelineCreateInfo);
}

JseResult JseGfxCore::DeleteGraphicsPipeline(JseGrapicsPipelineID pipelineId)
{
    return DeleteGraphicsPipeline_impl(pipelineId);
}

JseResult JseGfxCore::BindGraphicsPipeline(JseGrapicsPipelineID pipelineId)
{
    return BindGraphicsPipeline_impl(pipelineId);
}

JseResult JseGfxCore::CreateFrameBuffer(const JseFrameBufferCreateInfo& frameBufferCreateInfo)
{
    return CreateFrameBuffer_impl(frameBufferCreateInfo);
}

JseResult JseGfxCore::DeleteFrameBuffer(JseFrameBufferID framebufferId)
{
    return DeleteFrameBuffer_impl(framebufferId);
}

JseResult JseGfxCore::CreateShader(const JseShaderCreateInfo& shaderCreateInfo, std::string& errorOutput)
{
    return CreateShader_impl(shaderCreateInfo, errorOutput);
}

JseResult JseGfxCore::BeginRenderPass(const JseRenderPassInfo& renderPassInfo)
{
    return BeginRenderPass_impl(renderPassInfo);
}

JseResult JseGfxCore::EndRenderPass()
{
    return EndRenderPass_impl();
}

void JseGfxCore::BindVertexBuffer(uint32_t binding, JseBufferID buffer, JseDeviceSize offsets)
{
    BindVertexBuffer_impl(binding, buffer, offsets);
}

void JseGfxCore::BindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, const JseBufferID* pBuffers, const JseDeviceSize* pOffsets)
{
    BindVertexBuffers_impl(firstBinding, bindingCount, pBuffers, pOffsets);
}

void JseGfxCore::BindIndexBuffer(JseBufferID buffer, uint32_t offset, JseIndexType type)
{
    BindIndexBuffer_impl(buffer, offset, type);
}

JseResult JseGfxCore::GetDeviceCapabilities(JseDeviceCapabilities& dest)
{
    return GetDeviceCapabilities_impl(dest);
}

JseResult JseGfxCore::SetVSyncInterval(int interval)
{
    return SetVSyncInterval_impl(interval);
}

JseResult JseGfxCore::GetSurfaceDimension(JseRect2D& x)
{
    return GetSurfaceDimension_impl(x);
}

void JseGfxCore::Shutdown()
{
    Shutdown_impl();
}
