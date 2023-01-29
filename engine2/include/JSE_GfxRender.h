#ifndef JSE_GFX_RENDER_H
#define JSE_GFX_RENDER_H

#include <stdexcept>

enum RenderCommand {
	RC_NOP = 0,
	RC_BEGIN_RENDERPASS,
	RC_CREATE_GRAPHICS_PIPELINE,
	RC_VIEWPORT,
	RC_SCISSOR,
	RC_CREATE_SHADER,
	RC_CREATE_DESCRIPTOR_SET_LAYOUT_BINDING,
	RC_CREATE_DESCRIPTOR_SET,
	RC_WRITE_DESCRIPTOR_SET,
	RC_BIND_DESCRIPTOR_SETS,
	RC_CREATE_BUFFER,
	RC_UPDATE_BUFFER,
	RC_CREATE_IMAGE,
	RC_UPLOAD_IMAGE,
	RC_BIND_GRAPHICS_PIPELINE,
	RC_BIND_VERTEX_BUFFERS,
	RC_DRAW
};

struct JseCmdEmpty {
	RenderCommand command{ RC_NOP };
	RenderCommand* next;
};
struct JseCmdBeginRenderpass {
	RenderCommand command{ RC_BEGIN_RENDERPASS };
	RenderCommand* next;
	JseRenderPassInfo info;
};
struct JseCmdCreateGraphicsPipeline {
	RenderCommand command{ RC_CREATE_GRAPHICS_PIPELINE };
	RenderCommand* next;
	JseGraphicsPipelineCreateInfo info;
};
struct JseCmdCreateDescriptorSetLayoutBindind {
	RenderCommand command{ RC_CREATE_DESCRIPTOR_SET_LAYOUT_BINDING };
	RenderCommand* next;
	JseDescriptorSetLayoutCreateInfo info;
};
struct JseCmdViewport {
	RenderCommand command{ RC_VIEWPORT };
	RenderCommand* next;
	JseRect2D viewport;
};
struct JseCmdScissor {
	RenderCommand command{ RC_SCISSOR };
	RenderCommand* next;
	JseRect2D scissor;
};
struct JseCmdCreateShader {
	RenderCommand command{ RC_CREATE_SHADER };
	RenderCommand* next;
	JseShaderCreateInfo info;
};
struct JseCmdCreateBuffer {
	RenderCommand command{ RC_CREATE_BUFFER };
	RenderCommand* next;
	JseBufferCreateInfo info;
};
struct JseCmdUpdateBuffer {
	RenderCommand command{ RC_UPDATE_BUFFER };
	RenderCommand* next;
	JseBufferUpdateInfo info;
};
struct JseCmdBindGraphicsPipeline {
	RenderCommand command{ RC_BIND_GRAPHICS_PIPELINE };
	RenderCommand* next;
	JseGrapicsPipelineID pipeline;
};
struct JseCmdBindVertexBuffers {
	RenderCommand command{ RC_BIND_VERTEX_BUFFERS };
	RenderCommand* next;
	uint32_t firstBinding; 
	uint32_t bindingCount; 
	JseBufferID* pBuffers;
	JseDeviceSize* pOffsets;
	JseGrapicsPipelineID pipeline;
};
struct JseCmdDraw {
	RenderCommand command{ RC_DRAW };
	RenderCommand* next;
	JseTopology mode;
	uint32_t vertexCount;
	uint32_t instanceCount;
	uint32_t firstVertex;
	uint32_t firstInstance;
};
struct JseCmdCreateImage {
	RenderCommand command{ RC_CREATE_IMAGE };
	RenderCommand* next;
	JseImageCreateInfo info;
};
struct JseCmdUploadImage {
	RenderCommand command{ RC_UPLOAD_IMAGE };
	RenderCommand* next;
	JseImageUploadInfo info;
};
struct JseCmdCreateDescriptorSet {
	RenderCommand command{ RC_CREATE_DESCRIPTOR_SET };
	RenderCommand* next;
	JseDescriptorSetCreateInfo info;
};
struct JseCmdWriteDescriptorSet {
	RenderCommand command{ RC_WRITE_DESCRIPTOR_SET };
	RenderCommand* next;
	JseWriteDescriptorSet info;
};
struct JseCmdBindDescriptorSets {
	RenderCommand command{ RC_BIND_DESCRIPTOR_SETS };
	RenderCommand* next;
	uint32_t firstSet; 
	uint32_t descriptorSetCount;
	uint32_t dynamicOffsetCount; 
	uint32_t* pDynamicOffsets;
	JseDescriptorSetID* pDescriptorSets;
};

class JseGfxRenderer {
public:
	static const size_t FRAME_MEM_SIZE = 128 * 1024 * 1024;
	static const size_t ON_FLIGHT_FRAMES = 2;
private:
	struct frameData_t {
		JseAtomicInt				frameMemoryPtr;
		std::shared_ptr<uint8_t>	frameMemory;
		JseCmdEmpty*				cmdTail;
		JseCmdEmpty*				cmdHead;
	};

	JseGfxCore*				core_;
	JseThread				renderThread_;
	JseMutex				renderThreadMtx_;
	JseConditionVariable	renderThreadSync_;
	bool					renderThreadReady_;
	bool					renderThreadDoWork_;
	JseAtomicInt			shouldTerminate_{ 0 };
	size_t					frameMemorySize_{};
	int						maxFrameMemUsage_{ 0 };
	frameData_t				frames_[ON_FLIGHT_FRAMES];
	int						activeFrame_;
	int						renderFrame_;
	frameData_t*			frameData_;
	frameData_t*			renderData_;
	bool					useThread_{};
	bool					threadRunning_{};
	bool					initialized_{};
	
	void ResetCommandBuffer();

public: 
	JseGfxRenderer();
	~JseGfxRenderer();

	JseGfxCore* GetCore();

	void SetCore(JseGfxCore* core);

	JseResult InitCore(int w, int h, bool fs, bool useThread);
	
	void* GetMappedBufferPointer(JseBufferID id);

	uint8_t* R_FrameAlloc(uint32_t bytes);

	template<typename _Ty>
	_Ty* FrameAlloc(int count = 1) {
		uint32_t bytes = sizeof(_Ty) * count;
		uint8_t* pData = R_FrameAlloc(bytes);

		return reinterpret_cast<_Ty*>(pData);
	}

	template<typename _Type>
	_Type* GetCommandBuffer() {
		_Type* cmd;
		cmd = new (R_FrameAlloc(sizeof(_Type))) _Type();
		cmd->next = nullptr;
		frameData_->cmdTail->next = &cmd->command;
		frameData_->cmdTail = RCAST(JseCmdEmpty*, cmd);

		return cmd;
	}

	template<typename _Ty>
	void GetCommandBuffer(_Ty** ref) {
		_Ty* cmd = new (R_FrameAlloc(sizeof(_Ty))) _Ty();
		cmd->next = nullptr;
		frameData_->cmdTail->next = &cmd->command;
		frameData_->cmdTail = RCAST(JseCmdEmpty*, cmd);

		*ref = cmd;
	}

	void Frame();
	void RenderFrame(frameData_t* renderData);
	void ProcessCommandList(frameData_t* frameData);
	void RenderThread();
	static int RenderThreadWrapper(void* data);
};

#endif