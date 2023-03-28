#include "JSE.h"

namespace js
{
    Result GfxCore::Init(bool debugMode)
    {
        return Init_impl(debugMode);
    }

    Result GfxCore::CreateSurface(const JseSurfaceCreateInfo& createSurfaceInfo)
    {
        if (createSurfaceInfo.width == 0 && createSurfaceInfo.height == 0 && createSurfaceInfo.fullScreen == false) {
            return Result::INVALID_SURFACE_DIMENSION;
        }

        return CreateSurface_impl(createSurfaceInfo);
    }

    Result GfxCore::CreateBuffer(const JseBufferCreateInfo& createBufferInfo)
    {
        return CreateBuffer_impl(createBufferInfo);
    }

    Result GfxCore::UpdateBuffer(const JseBufferUpdateInfo& bufferUpdateInfo)
    {
        return UpdateBuffer_impl(bufferUpdateInfo);
    }

    Result GfxCore::DestroyBuffer(JseBufferID bufferId)
    {
        return DestroyBuffer_impl(bufferId);
    }

    Result GfxCore::CreateImage(const JseImageCreateInfo& createImageInfo)
    {
        return CreateImage_impl(createImageInfo);
    }

    Result GfxCore::CreateTexture(const JseImageCreateInfo& createImageInfo)
    {
        return CreateImage_impl(createImageInfo);
    }

    Result GfxCore::UpdateImageData(const JseImageUploadInfo& imageUploadInfo)
    {
        return UpdateImageData_impl(imageUploadInfo);
    }

    Result GfxCore::DeleteImage(JseImageID imageId)
    {
        return DeleteImage_impl(imageId);
    }

    Result GfxCore::CreateGraphicsPipeline(const JseGraphicsPipelineCreateInfo& graphicsPipelineCreateInfo)
    {
        return CreateGraphicsPipeline_impl(graphicsPipelineCreateInfo);
    }

    Result GfxCore::DeleteGraphicsPipeline(JseGrapicsPipelineID pipelineId)
    {
        return DeleteGraphicsPipeline_impl(pipelineId);
    }

    Result GfxCore::BindGraphicsPipeline(JseGrapicsPipelineID pipelineId)
    {
        return BindGraphicsPipeline_impl(pipelineId);
    }

    Result GfxCore::CreateFrameBuffer(const JseFrameBufferCreateInfo& frameBufferCreateInfo)
    {
        return CreateFrameBuffer_impl(frameBufferCreateInfo);
    }

    Result GfxCore::DeleteFrameBuffer(JseFrameBufferID framebufferId)
    {
        return DeleteFrameBuffer_impl(framebufferId);
    }

    Result GfxCore::GenShader(const JseShaderCreateInfo& shaderCreateInfo, std::string& errorOutput)
    {
        return CreateShader_impl(shaderCreateInfo, errorOutput);
    }

    Result GfxCore::BeginRenderPass(const JseRenderPassInfo& renderPassInfo)
    {
        return BeginRenderPass_impl(renderPassInfo);
    }

    Result GfxCore::CreateDescriptorSetLayout(const JseDescriptorSetLayoutCreateInfo& cmd)
    {
        return CreateDescriptorSetLayout_impl(cmd);
    }

    Result GfxCore::EndRenderPass()
    {
        return EndRenderPass_impl();
    }

    Result GfxCore::CreateDescriptorSet(const JseDescriptorSetCreateInfo& cmd)
    {
        return CreateDescriptorSet_impl(cmd);
    }

    Result GfxCore::WriteDescriptorSet(const JseWriteDescriptorSet& cmd)
    {
        return WriteDescriptorSet_impl(cmd);
    }

    Result GfxCore::BindDescriptorSet(uint32_t firstSet, uint32_t descriptorSetCount, const JseDescriptorSetID* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets)
    {
        return BindDescriptorSet_impl(firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    }

    Result GfxCore::CreateFence(JseFenceID id)
    {
        return CreateFence_impl(id);
    }

    Result GfxCore::DeleteFence(JseFenceID id)
    {
        return DeleteFence_impl(id);
    }

    Result GfxCore::WaitSync(JseFenceID id, uint64_t timeout)
    {
        return WaitSync_impl(id, timeout);
    }

    void GfxCore::SwapChainNextImage()
    {
        SwapChainNextImage_impl();
    }

    void GfxCore::BeginRendering()
    {
        BeginRendering_impl();
    }

    void GfxCore::EndRendering()
    {
        EndRendering_impl();
    }

    void GfxCore::BindVertexBuffer(uint32_t binding, JseBufferID buffer, JseDeviceSize offsets)
    {
        BindVertexBuffer_impl(binding, buffer, offsets);
    }

    void GfxCore::BindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, const JseBufferID* pBuffers, const JseDeviceSize* pOffsets)
    {
        BindVertexBuffers_impl(firstBinding, bindingCount, pBuffers, pOffsets);
    }

    void GfxCore::BindIndexBuffer(JseBufferID buffer, uint32_t offset, JseIndexType type)
    {
        BindIndexBuffer_impl(buffer, offset, type);
    }

    void GfxCore::Draw(JseTopology mode, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
        Draw_impl(mode, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void GfxCore::DrawIndexed(JseTopology mode, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance)
    {
        DrawIndexed_impl(mode, indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
    }

    void GfxCore::Viewport(const JseRect2D& x)
    {
        Viewport_impl(x);
    }

    void GfxCore::Scissor(const JseRect2D& x)
    {
        Scissor(x);
    }

    void* GfxCore::GetMappedBufferPointer(JseBufferID id) {
        return GetMappedBufferPointer_impl(id);
    }

    Result GfxCore::GetDeviceCapabilities(JseDeviceCapabilities& dest)
    {
        return GetDeviceCapabilities_impl(dest);
    }

    Result GfxCore::SetVSyncInterval(int interval)
    {
        return SetVSyncInterval_impl(interval);
    }

    Result GfxCore::GetSurfaceDimension(glm::ivec2& x)
    {
        return GetSurfaceDimension_impl(x);
    }

    void GfxCore::Shutdown()
    {
        Shutdown_impl();
    }
}