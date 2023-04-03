#ifndef JSE_GFX_CORE_NULL
#define JSE_GFX_CORE_NULL

#include "JSE.h"
namespace js {
	class GfxCoreNull : public js::GfxCore
	{
	public:
		GfxCoreNull();
	private:
		// Inherited via JseGfxCore
		virtual void* GetMappedBufferPointer_impl(JseBufferID id) override;
		virtual Result Init_impl(bool debugMode) override;
		virtual Result CreateSurface_impl(const JseSurfaceCreateInfo& createSurfaceInfo) override;
		virtual Result CreateBuffer_impl(const JseBufferCreateInfo& createBufferInfo) override;
		virtual Result UpdateBuffer_impl(const JseBufferUpdateInfo& bufferUpdateInfo) override;
		virtual Result DestroyBuffer_impl(JseBufferID bufferId) override;
		virtual Result CreateImage_impl(const JseImageCreateInfo& createImageInfo) override;
		virtual Result DeleteImage_impl(JseImageID imageId) override;
		virtual Result CreateGraphicsPipeline_impl(const JseGraphicsPipelineCreateInfo& graphicsPipelineCreateInfo) override;
		virtual Result BindGraphicsPipeline_impl(JseGrapicsPipelineID pipelineId) override;
		virtual Result DeleteGraphicsPipeline_impl(JseGrapicsPipelineID pipelineId) override;
		virtual Result GetDeviceCapabilities_impl(JseDeviceCapabilities& dest) override;
		virtual Result UpdateImageData_impl(const JseImageUploadInfo& imgageUploadInfo) override;
		virtual Result CreateShader_impl(const JseShaderCreateInfo& shaderCreateInfo, std::string& errorOutput) override;
		virtual Result CreateFrameBuffer_impl(const JseFrameBufferCreateInfo& frameBufferCreateInfo) override;
		virtual Result DeleteFrameBuffer_impl(JseFrameBufferID framebufferId) override;
		virtual Result BeginRenderPass_impl(const JseRenderPassInfo& renderPassInfo) override;
		virtual Result CreateDescriptorSetLayout_impl(const JseDescriptorSetLayoutCreateInfo& cmd) override;
		virtual Result BindDescriptorSet_impl(uint32_t firstSet, uint32_t descriptorSetCount, const JseDescriptorSetID* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) override;
		virtual Result EndRenderPass_impl() override;
		virtual Result CreateDescriptorSet_impl(const JseDescriptorSetCreateInfo& cmd) override;
		virtual Result WriteDescriptorSet_impl(const JseWriteDescriptorSet& cmd) override;
		virtual Result CreateFence_impl(JseFenceID id) override;
		virtual Result DeleteFence_impl(JseFenceID id) override;
		virtual Result WaitSync_impl(JseFenceID id, uint64_t time) override;

		virtual void BindVertexBuffers_impl(uint32_t firstBinding, uint32_t bindingCount, const JseBufferID* pBuffers, const JseDeviceSize* pOffsets) override;
		virtual void BindVertexBuffer_impl(uint32_t binding, JseBufferID buffer, JseDeviceSize offsets) override;
		virtual void BindIndexBuffer_impl(JseBufferID buffer, uint32_t offset, JseIndexType type) override;
		virtual void Draw_impl(JseTopology mode, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;
		virtual void DrawIndexed_impl(JseTopology mode, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) override;
		virtual void Viewport_impl(const JseRect2D& x) override;
		virtual void Scissor_impl(const JseRect2D& x) override;
		virtual void BeginRendering_impl() override;
		virtual void EndRendering_impl() override;
		virtual void SwapChainNextImage_impl() override;

		virtual Result SetVSyncInterval_impl(int interval) override;
		virtual Result GetSurfaceDimension_impl(glm::ivec2& x) override;
		virtual void Shutdown_impl() override;
	};
}
#endif // !JSE_GFX_CORE_NULL
