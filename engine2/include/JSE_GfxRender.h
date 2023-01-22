#ifndef JSE_GFX_RENDER_H
#define JSE_GFX_RENDER_H

#include <stdexcept>

enum RenderCommand {
	RC_CreateBufer,
	RC_CreateTexture,
	RC_UpdateTextureData,
	RC_DeleteTexture,
	RC_CreateGraphicsPipeline,
	RC_DeleteGraphicsPipeline,
	RC_BindGraphicsPipeline,
	RC_CreateFrameBuffer,
	RC_DeleteFrameBuffer,
	RC_CreateShader,
	RC_BeginRenderPass,
	RC_CreateDescriptorSetLayout,
	RC_EndRenderPass,
	RC_CreateDescriptorSet,
	RC_WriteDescriptorSet,
	RC_BindDescriptorSet,
	RC_BindVertexBuffer,
	RC_BindVertexBuffers,
	RC_BindIndexBuffer,
	RC_Draw,
	RC_DrawIndexed,
	RC_SetSwapInterval,
	RC_NOP
};

struct JseGfxCmdCreateBuffer {
	RenderCommand command{ RC_CreateBufer };
	RenderCommand* next;
	JseBufferCreateInfo info;
};

struct JseGfxCmdCreateTexture {
	RenderCommand command{ RC_CreateTexture };
	RenderCommand* next;
	JseImageUploadInfo info;
};

struct JseGfxCmdUpdateTextureData {
	RenderCommand command{ RC_UpdateTextureData };
	RenderCommand* next;
	JseImageUploadInfo info;
};

struct JseGfxCmdDeleteTexture {
	RenderCommand command{ RC_DeleteTexture };
	RenderCommand* next;
	JseImageID image;
};

struct JseGfxCmdCreateGraphicsPipeline {
	RenderCommand command{ RC_CreateGraphicsPipeline };
	RenderCommand* next;
	JseGraphicsPipelineCreateInfo info;
};

struct JseGfxCmdDeleteGraphicsPipeline {
	RenderCommand command{ RC_DeleteGraphicsPipeline };
	RenderCommand* next;
	JseGrapicsPipelineID pipeline;
};

struct JseGfxCmdBindGraphicsPipeline {
	RenderCommand command{ RC_BindGraphicsPipeline };
	RenderCommand* next;
	JseGrapicsPipelineID pipeline;
};

struct JseGfxCmdCreateFrameBuffer {
	RenderCommand command{ RC_CreateFrameBuffer };
	RenderCommand* next;
	JseFrameBufferCreateInfo info;
};

struct JseGfxCmdDeleteFrameBuffer {
	RenderCommand command{ RC_DeleteFrameBuffer };
	RenderCommand* next;
	JseFrameBufferID framebuffer;
};

struct JseGfxCmdCreateShader {
	RenderCommand command{ RC_CreateShader };
	RenderCommand* next;
	JseShaderCreateInfo info;
};

struct JseGfxCmdBeginRenderPass {
	RenderCommand command{ RC_BeginRenderPass };
	RenderCommand* next;
	JseRenderPassInfo info;
};

struct JseGfxCmdCreateDescriptorSetLayout {
	RenderCommand command{ RC_CreateDescriptorSetLayout };
	RenderCommand* next;
	JseDescriptorSetLayoutCreateInfo info;
};

struct JseGfxCmdEndRenderPass {
	RenderCommand command{ RC_EndRenderPass };
	RenderCommand* next;
};

struct JseGfxCmdCreateDescriptorSet {
	RenderCommand command{ RC_CreateDescriptorSet };
	RenderCommand* next;
	JseDescriptorSetCreateInfo info;
};

struct JseGfxCmdWriteDescriptorSet {
	RenderCommand command{ RC_WriteDescriptorSet };
	RenderCommand* next;
	JseWriteDescriptorSet info;
};

struct JseGfxCmdBindDescriptorSet {
	RenderCommand command{ RC_BindDescriptorSet };
	RenderCommand* next;
	uint32_t firstSet;
	uint32_t descriptorSetCount;
	const JseDescriptorSetID* pDescriptorSets; 
	uint32_t dynamicOffsetCount; 
	const uint32_t* pDynamicOffsets;
};

struct JseGfxCmdBindVertexBuffer {
	RenderCommand command{ RC_BindIndexBuffer };
	RenderCommand* next;
	uint32_t binding;
	JseBufferID buffer; 
	JseDeviceSize offsets;
};

struct JseGfxCmdBindVertexBuffers {
	RenderCommand command{ RC_BindIndexBuffer };
	RenderCommand* next;
	uint32_t firstBinding;
	uint32_t bindingCount;
	const JseBufferID* pBuffers;
	const JseDeviceSize* pOffsets;
};

struct JseGfxCmdBindIndexBuffer {
	RenderCommand command{ RC_BindIndexBuffer };
	RenderCommand* next;
	JseBufferID buffer;
	uint32_t offset;
	JseIndexType type;
};

struct JseGfxCmdDraw {
	RenderCommand command{ RC_Draw };
	RenderCommand* next;
	JseTopology mode;
	uint32_t vertexCount;
	uint32_t instanceCount;
	uint32_t firstVertex;
	uint32_t firstInstance;
};

struct JseGfxCmdDrawIndexed {
	RenderCommand command{ RC_DrawIndexed };
	RenderCommand* next;
	JseTopology mode;
	uint32_t indexCount;
	uint32_t instanceCount;
	uint32_t firstIndex;
	int32_t vertexOffset;
	uint32_t firstInstance;
};

struct JseGfxCmdSetVSyncInterval {
	RenderCommand command{ RC_SetSwapInterval };
	RenderCommand* next;
	int interval;
};

struct JseGfxCmdEmpty {
	RenderCommand command{ RC_NOP };
	RenderCommand* next;
};

class JseGfxRenderer {
public:
	static const size_t FRAME_MEM_SIZE = 128 * 1024 * 1024;
	static const size_t ON_FLIGHT_FRAMES = 2;
private:
	struct frameData_t {
		JseAtomicInt frameMemoryPtr;
		std::shared_ptr<uint8_t> frameMemory;
		JseGfxCmdEmpty* cmdTail;
		JseGfxCmdEmpty* cmdHead;
	};

	JseGfxCore*		core;
	JseSemaphore	renderSem_{0};
	JseSemaphore	frontendSem_{0};
	size_t			frameMemorySize_{};
	int				maxFrameMemUsage_{ 0 };
	frameData_t		frames_[ON_FLIGHT_FRAMES];
	int				activeFrame_;
	int				renderFrame_;
	frameData_t*	frameData_;

	uint32_t CPU_CACHELINE_SIZE{};

public: 
	JseGfxRenderer();
	~JseGfxRenderer();

	void CreateBuffer(const JseBufferCreateInfo& info, std::promise<JseResult> result);

	uint8_t* R_FrameAlloc(uint32_t bytes);

	template<class _Ty>
	_Ty* FrameAlloc() {
		uint32_t bytes = sizeof(_Ty);
		uint8_t* pData = R_FrameAlloc(bytes);

		return reinterpret_cast<_Ty*>(pData);
	}

	template<class _Ty>
	_Ty* GetCommandBuffer()
	{
		_Ty* cmd;
		cmd = reinterpret_cast<_Ty*>(R_FrameAlloc(sizeof(_Ty)));
		cmd->next = nullptr;
		frameData_->cmdTail->next = &cmd->command;
		frameData_->cmdTail = (JseGfxCmdEmpty*)cmd;

		return cmd;
	}

	void Frame();

};

#endif