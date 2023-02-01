#include "JSE_GfxCoreNull.h"

JseGfxCoreNull::JseGfxCoreNull()
{
    Info("JseGfxCore NULL Driver v0.1");
}

void* JseGfxCoreNull::GetMappedBufferPointer_impl(JseBufferID id)
{
    return nullptr;
}

JseResult JseGfxCoreNull::Init_impl(bool debugMode)
{
    return JseResult();
}

JseResult JseGfxCoreNull::CreateSurface_impl(const JseSurfaceCreateInfo& createSurfaceInfo)
{
    return JseResult();
}

JseResult JseGfxCoreNull::CreateBuffer_impl(const JseBufferCreateInfo& createBufferInfo)
{
    return JseResult();
}

JseResult JseGfxCoreNull::UpdateBuffer_impl(const JseBufferUpdateInfo& bufferUpdateInfo)
{
    return JseResult();
}

JseResult JseGfxCoreNull::DestroyBuffer_impl(JseBufferID bufferId)
{
    return JseResult();
}

JseResult JseGfxCoreNull::CreateImage_impl(const JseImageCreateInfo& createImageInfo)
{
    return JseResult();
}

JseResult JseGfxCoreNull::DeleteImage_impl(JseImageID imageId)
{
    return JseResult();
}

JseResult JseGfxCoreNull::CreateGraphicsPipeline_impl(const JseGraphicsPipelineCreateInfo& graphicsPipelineCreateInfo)
{
    return JseResult();
}

JseResult JseGfxCoreNull::BindGraphicsPipeline_impl(JseGrapicsPipelineID pipelineId)
{
    return JseResult();
}

JseResult JseGfxCoreNull::DeleteGraphicsPipeline_impl(JseGrapicsPipelineID pipelineId)
{
    return JseResult();
}

JseResult JseGfxCoreNull::GetDeviceCapabilities_impl(JseDeviceCapabilities& dest)
{
    return JseResult();
}

JseResult JseGfxCoreNull::UpdateImageData_impl(const JseImageUploadInfo& imgageUploadInfo)
{
    return JseResult();
}

JseResult JseGfxCoreNull::CreateShader_impl(const JseShaderCreateInfo& shaderCreateInfo, std::string& errorOutput)
{
    return JseResult();
}

JseResult JseGfxCoreNull::CreateFrameBuffer_impl(const JseFrameBufferCreateInfo& frameBufferCreateInfo)
{
    return JseResult();
}

JseResult JseGfxCoreNull::DeleteFrameBuffer_impl(JseFrameBufferID framebufferId)
{
    return JseResult();
}

JseResult JseGfxCoreNull::BeginRenderPass_impl(const JseRenderPassInfo& renderPassInfo)
{
    return JseResult();
}

JseResult JseGfxCoreNull::CreateDescriptorSetLayout_impl(const JseDescriptorSetLayoutCreateInfo& cmd)
{
    return JseResult();
}

JseResult JseGfxCoreNull::BindDescriptorSet_impl(uint32_t firstSet, uint32_t descriptorSetCount, const JseDescriptorSetID* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets)
{
    return JseResult();
}

JseResult JseGfxCoreNull::EndRenderPass_impl()
{
    return JseResult();
}

JseResult JseGfxCoreNull::CreateDescriptorSet_impl(const JseDescriptorSetCreateInfo& cmd)
{
    return JseResult();
}

JseResult JseGfxCoreNull::WriteDescriptorSet_impl(const JseWriteDescriptorSet& cmd)
{
    return JseResult();
}

void JseGfxCoreNull::BindVertexBuffers_impl(uint32_t firstBinding, uint32_t bindingCount, const JseBufferID* pBuffers, const JseDeviceSize* pOffsets)
{
}

void JseGfxCoreNull::BindVertexBuffer_impl(uint32_t binding, JseBufferID buffer, JseDeviceSize offsets)
{
}

void JseGfxCoreNull::BindIndexBuffer_impl(JseBufferID buffer, uint32_t offset, JseIndexType type)
{
}

void JseGfxCoreNull::Draw_impl(JseTopology mode, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
}

void JseGfxCoreNull::DrawIndexed_impl(JseTopology mode, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
}

void JseGfxCoreNull::Viewport_impl(const JseRect2D& x)
{
}

void JseGfxCoreNull::Scissor_impl(const JseRect2D& x)
{
}

void JseGfxCoreNull::BeginRendering_impl()
{
}

void JseGfxCoreNull::EndRendering_impl()
{
}

void JseGfxCoreNull::SwapChainNextImage_impl()
{
}

JseResult JseGfxCoreNull::SetVSyncInterval_impl(int interval)
{
    return JseResult();
}

JseResult JseGfxCoreNull::GetSurfaceDimension_impl(glm::ivec2& x)
{
    return JseResult();
}

void JseGfxCoreNull::Shutdown_impl()
{
}
