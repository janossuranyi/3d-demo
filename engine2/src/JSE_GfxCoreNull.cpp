#include "JSE_GfxCoreNull.h"

namespace js {
    GfxCoreNull::GfxCoreNull()
    {
        Info("GfxCore NULL Driver v0.1");
    }

    void* GfxCoreNull::GetMappedBufferPointer_impl(JseBufferID id)
    {
        return nullptr;
    }

    Result GfxCoreNull::Init_impl(bool debugMode)
    {
        return Result();
    }

    Result GfxCoreNull::CreateSurface_impl(const JseSurfaceCreateInfo& createSurfaceInfo)
    {
        return Result();
    }

    Result GfxCoreNull::CreateBuffer_impl(const JseBufferCreateInfo& createBufferInfo)
    {
        return Result();
    }

    Result GfxCoreNull::UpdateBuffer_impl(const JseBufferUpdateInfo& bufferUpdateInfo)
    {
        return Result();
    }

    Result GfxCoreNull::DestroyBuffer_impl(JseBufferID bufferId)
    {
        return Result();
    }

    Result GfxCoreNull::CreateImage_impl(const JseImageCreateInfo& createImageInfo)
    {
        return Result();
    }

    Result GfxCoreNull::DeleteImage_impl(JseImageID imageId)
    {
        return Result();
    }

    Result GfxCoreNull::CreateGraphicsPipeline_impl(const JseGraphicsPipelineCreateInfo& graphicsPipelineCreateInfo)
    {
        return Result();
    }

    Result GfxCoreNull::BindGraphicsPipeline_impl(JseGrapicsPipelineID pipelineId)
    {
        return Result();
    }

    Result GfxCoreNull::DeleteGraphicsPipeline_impl(JseGrapicsPipelineID pipelineId)
    {
        return Result();
    }

    Result GfxCoreNull::GetDeviceCapabilities_impl(JseDeviceCapabilities& dest)
    {
        return Result();
    }

    Result GfxCoreNull::UpdateImageData_impl(const JseImageUploadInfo& imgageUploadInfo)
    {
        return Result();
    }

    Result GfxCoreNull::CreateShader_impl(const JseShaderCreateInfo& shaderCreateInfo, std::string& errorOutput)
    {
        return Result();
    }

    Result GfxCoreNull::CreateFrameBuffer_impl(const JseFrameBufferCreateInfo& frameBufferCreateInfo)
    {
        return Result();
    }

    Result GfxCoreNull::DeleteFrameBuffer_impl(JseFrameBufferID framebufferId)
    {
        return Result();
    }

    Result GfxCoreNull::BeginRenderPass_impl(const JseRenderPassInfo& renderPassInfo)
    {
        return Result();
    }

    Result GfxCoreNull::CreateDescriptorSetLayout_impl(const JseDescriptorSetLayoutCreateInfo& cmd)
    {
        return Result();
    }

    Result GfxCoreNull::BindDescriptorSet_impl(uint32_t firstSet, uint32_t descriptorSetCount, const JseDescriptorSetID* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets)
    {
        return Result();
    }

    Result GfxCoreNull::EndRenderPass_impl()
    {
        return Result();
    }

    Result GfxCoreNull::CreateDescriptorSet_impl(const JseDescriptorSetCreateInfo& cmd)
    {
        return Result();
    }

    Result GfxCoreNull::WriteDescriptorSet_impl(const JseWriteDescriptorSet& cmd)
    {
        return Result();
    }

    Result GfxCoreNull::CreateFence_impl(JseFenceID id)
    {
        return Result();
    }

    Result GfxCoreNull::DeleteFence_impl(JseFenceID id)
    {
        return Result();
    }

    Result GfxCoreNull::WaitSync_impl(JseFenceID id, uint64_t time)
    {
        return Result();
    }

    void GfxCoreNull::BindVertexBuffers_impl(uint32_t firstBinding, uint32_t bindingCount, const JseBufferID* pBuffers, const JseDeviceSize* pOffsets)
    {
    }

    void GfxCoreNull::BindVertexBuffer_impl(uint32_t binding, JseBufferID buffer, JseDeviceSize offsets)
    {
    }

    void GfxCoreNull::BindIndexBuffer_impl(JseBufferID buffer, uint32_t offset, JseIndexType type)
    {
    }

    void GfxCoreNull::Draw_impl(JseTopology mode, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
    }

    void GfxCoreNull::DrawIndexed_impl(JseTopology mode, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
    {
    }

    void GfxCoreNull::Viewport_impl(const JseRect2D& x)
    {
    }

    void GfxCoreNull::Scissor_impl(const JseRect2D& x)
    {
    }

    void GfxCoreNull::BeginRendering_impl()
    {
    }

    void GfxCoreNull::EndRendering_impl()
    {
    }

    void GfxCoreNull::SwapChainNextImage_impl()
    {
    }

    Result GfxCoreNull::SetVSyncInterval_impl(int interval)
    {
        return Result();
    }

    Result GfxCoreNull::GetSurfaceDimension_impl(glm::ivec2& x)
    {
        return Result();
    }

    void GfxCoreNull::Shutdown_impl()
    {
    }
}