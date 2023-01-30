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

struct JseCmdEmpty {};
struct JseCmdBeginRenderpass {
	JseRenderPassInfo info;
};
struct JseCmdCreateGraphicsPipeline {
	JseGraphicsPipelineCreateInfo info;
};
struct JseCmdCreateDescriptorSetLayoutBindind {
	JseDescriptorSetLayoutCreateInfo info;
};
struct JseCmdViewport {
	JseRect2D viewport;
};
struct JseCmdScissor {
	JseRect2D scissor;
};
struct JseCmdCreateShader {
	JseShaderCreateInfo info;
};
struct JseCmdCreateBuffer {
	JseBufferCreateInfo info;
};
struct JseCmdUpdateBuffer {
	JseBufferUpdateInfo info;
};
struct JseCmdBindGraphicsPipeline {
	JseGrapicsPipelineID pipeline;
};
struct JseCmdBindVertexBuffers {
	uint32_t firstBinding; 
	uint32_t bindingCount; 
	JseBufferID* pBuffers;
	JseDeviceSize* pOffsets;
	JseGrapicsPipelineID pipeline;
};
struct JseCmdDraw {
	JseTopology mode;
	uint32_t vertexCount;
	uint32_t instanceCount;
	uint32_t firstVertex;
	uint32_t firstInstance;
};
struct JseCmdCreateImage {
	JseImageCreateInfo info;
};
struct JseCmdUploadImage {
	JseImageUploadInfo info;
};
struct JseCmdCreateDescriptorSet {
	JseDescriptorSetCreateInfo info;
};
struct JseCmdWriteDescriptorSet {
	JseWriteDescriptorSet info;
};
struct JseCmdBindDescriptorSets {
	uint32_t firstSet; 
	uint32_t descriptorSetCount;
	uint32_t dynamicOffsetCount; 
	uint32_t* pDynamicOffsets;
	JseDescriptorSetID* pDescriptorSets;
};

using JseCmd = std::variant<
	JseCmdEmpty,
	JseCmdBindDescriptorSets,
	JseCmdWriteDescriptorSet,
	JseCmdCreateDescriptorSet,
	JseCmdUploadImage,
	JseCmdCreateImage,
	JseCmdDraw,
	JseCmdBindVertexBuffers,
	JseCmdBindGraphicsPipeline,
	JseCmdUpdateBuffer,
	JseCmdCreateBuffer,
	JseCmdCreateShader,
	JseCmdScissor,
	JseCmdViewport,
	JseCmdCreateDescriptorSetLayoutBindind,
	JseCmdCreateGraphicsPipeline,
	JseCmdBeginRenderpass
>;

struct JseCmdWrapper {
	JseCmd command;
	JseCmdWrapper* next;
};

class JseGfxRenderer {
public:
	static const size_t DEFAULT_FRAME_MEM_SIZE = 64 * 1024 * 1024;
	static const size_t ON_FLIGHT_FRAMES = 2;
private:
	struct frameData_t {
		JseAtomicInt				frameMemoryPtr;
		std::shared_ptr<uint8_t>	frameMemory;
		JseCmdWrapper*				cmdTail;
		JseCmdWrapper*				cmdHead;
	};

	JseGfxCore*				core_;
	JseThread				renderThread_;
	JseMutex				renderThreadMtx_;
	JseConditionVariable	renderThreadSync_;
	bool					renderThreadReady_;
	bool					renderThreadDoWork_;
	JseAtomicInt			shouldTerminate_{ 0 };
	JseAtomicInt			nextId_{ 1 };
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

	JseResult lastResult_;
public:

	void operator()(const JseCmdEmpty& cmd);
	void operator()(const JseCmdBeginRenderpass& cmd);
	void operator()(const JseCmdCreateGraphicsPipeline& cmd);
	void operator()(const JseCmdViewport& cmd);
	void operator()(const JseCmdScissor& cmd);
	void operator()(const JseCmdCreateShader& cmd);
	void operator()(const JseCmdCreateDescriptorSetLayoutBindind& cmd);
	void operator()(const JseCmdCreateBuffer& cmd);
	void operator()(const JseCmdUpdateBuffer& cmd);
	void operator()(const JseCmdBindVertexBuffers& cmd);
	void operator()(const JseCmdBindGraphicsPipeline& cmd);
	void operator()(const JseCmdDraw& cmd);
	void operator()(const JseCmdCreateImage& cmd);
	void operator()(const JseCmdUploadImage& cmd);
	void operator()(const JseCmdCreateDescriptorSet& cmd);
	void operator()(const JseCmdWriteDescriptorSet& cmd);
	void operator()(const JseCmdBindDescriptorSets& cmd);
	
	template <typename T> void operator()(const T& c) {
		static_assert(!std::is_same<T, T>::value, "Unimplemented RenderCommand");
	}

	JseGfxRenderer();
	JseGfxRenderer(int frameMemorySize);
	~JseGfxRenderer();
	
	uint32_t NextID();
	
	JseGfxCore* GetCore();
	void ExecuteInCriticalSection(std::function<void()> func);

	JseResult CreateImage(const JseImageCreateInfo& x);
	JseResult UploadImage(const JseImageUploadInfo& x);

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

	JseCmdWrapper* GetCommandBuffer();

	void SubmitCommand(const JseCmd& cmd) { 
		auto* p = GetCommandBuffer();
		std::memcpy(&p->command, &cmd, sizeof(cmd));
		//p->command = std::move(cmd);
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