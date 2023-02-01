#ifndef JSE_GFX_CORE_NULL
#define JSE_GFX_CORE_NULL

#include "JSE.h"

class JseGfxCoreNull : public JseGfxCore
{
public:
	JseGfxCoreNull();
private:
	// Inherited via JseGfxCore
	virtual void* GetMappedBufferPointer_impl(JseBufferID id) override;
	virtual JseResult Init_impl(bool debugMode) override;
	virtual JseResult CreateSurface_impl(const JseSurfaceCreateInfo& createSurfaceInfo) override;
	virtual JseResult CreateBuffer_impl(const JseBufferCreateInfo& createBufferInfo) override;
	virtual JseResult UpdateBuffer_impl(const JseBufferUpdateInfo& bufferUpdateInfo) override;
	virtual JseResult DestroyBuffer_impl(JseBufferID bufferId) override;
	virtual JseResult CreateImage_impl(const JseImageCreateInfo& createImageInfo) override;
	virtual JseResult DeleteImage_impl(JseImageID imageId) override;
	virtual JseResult CreateGraphicsPipeline_impl(const JseGraphicsPipelineCreateInfo& graphicsPipelineCreateInfo) override;
	virtual JseResult BindGraphicsPipeline_impl(JseGrapicsPipelineID pipelineId) override;
	virtual JseResult DeleteGraphicsPipeline_impl(JseGrapicsPipelineID pipelineId) override;
	virtual JseResult GetDeviceCapabilities_impl(JseDeviceCapabilities& dest) override;
	virtual JseResult UpdateImageData_impl(const JseImageUploadInfo& imgageUploadInfo) override;
	virtual JseResult CreateShader_impl(const JseShaderCreateInfo& shaderCreateInfo, std::string& errorOutput) override;
	virtual JseResult CreateFrameBuffer_impl(const JseFrameBufferCreateInfo& frameBufferCreateInfo) override;
	virtual JseResult DeleteFrameBuffer_impl(JseFrameBufferID framebufferId) override;
	virtual JseResult BeginRenderPass_impl(const JseRenderPassInfo& renderPassInfo) override;
	virtual JseResult CreateDescriptorSetLayout_impl(const JseDescriptorSetLayoutCreateInfo& cmd) override;
	virtual JseResult BindDescriptorSet_impl(uint32_t firstSet, uint32_t descriptorSetCount, const JseDescriptorSetID* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) override;
	virtual JseResult EndRenderPass_impl() override;
	virtual JseResult CreateDescriptorSet_impl(const JseDescriptorSetCreateInfo& cmd) override;
	virtual JseResult WriteDescriptorSet_impl(const JseWriteDescriptorSet& cmd) override;
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

	virtual JseResult SetVSyncInterval_impl(int interval) override;
	virtual JseResult GetSurfaceDimension_impl(glm::ivec2& x) override;
	virtual void Shutdown_impl() override;
};
#endif // !JSE_GFX_CORE_NULL
