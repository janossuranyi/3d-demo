#ifndef JSE_GFX_RENDER_H
#define JSE_GFX_RENDER_H

#include <stdexcept>

using Invokable = std::function<void()>;

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
struct JseCreateFenceCmd {
	JseFenceID id;
};
struct JseDeleteFenceCmd {
	JseFenceID id;
};
struct JseWaitSyncCmd {
	JseFenceID id;
	uint64_t timeout;
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
	JseCmdBeginRenderpass,
	JseCreateFenceCmd,
	JseDeleteFenceCmd,
	JseWaitSyncCmd
>;

struct JseCmdWrapper {
	JseCmd command;
	JseCmdWrapper* next;
};

class JseGfxRenderer : public JseModule {
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
	size_t					frameMemorySize_{ 0 };
	int						maxFrameMemUsage_{ 0 };
	frameData_t				frames_[ON_FLIGHT_FRAMES];
	frameData_t*			frameData_;
	frameData_t*			renderData_;
	int						activeFrame_;
	int						renderFrame_;
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
	void operator()(const JseCreateFenceCmd& cmd);
	void operator()(const JseDeleteFenceCmd& cmd);
	void operator()(const JseWaitSyncCmd& cmd);

	template <typename T> void operator()(const T& c) {
		static_assert(!std::is_same<T, T>::value, "Unimplemented RenderCommand");
	}

	JseGfxRenderer();
	JseGfxRenderer(int frameMemorySize);
	~JseGfxRenderer();
	
	JseType typeIndex() const override;

	uint32_t NextID();
	
	JseGfxCore*		GetCore();
	JseResult		CreateImage(const JseImageCreateInfo& x);
	JseResult		UploadImage(const JseImageUploadInfo& x);
	JseResult		InitCore(int w, int h, bool fs, bool useThread);
	
	uint8_t*		R_FrameAlloc(uint32_t bytes);
	JseCmdWrapper*	GetCommandBuffer();

	void	Invoke(Invokable func);
	void*	GetMappedBufferPointer(JseBufferID id);
	void	SubmitCommand(const JseCmd& cmd);
	void	SetCore(JseGfxCore* core);
	void	Frame();
	void	RenderFrame(frameData_t* renderData);
	void	ProcessCommandList(frameData_t* frameData);
	void	RenderThread();
	void	SetVSyncInterval(int x);
	JseResult WaitSync(JseFenceID id, uint64_t timeout);

	static int RenderThreadWrapper(void* data);

	template<typename T>
	T* FrameAlloc(int count = 1) {
		uint32_t bytes = sizeof(T) * count;
		uint8_t* pData = R_FrameAlloc(bytes);

		return reinterpret_cast<T*>(pData);
	}

	template<typename T> T* CreateCommand() {
		auto* ptr = GetCommandBuffer();
		ptr->command.emplace<T>();
		
		return RCAST(T*, &ptr->command);
	}
};

inline 	void JseGfxRenderer::SubmitCommand(const JseCmd& cmd) {
	auto* p = GetCommandBuffer();
	std::memcpy(&p->command, &cmd, sizeof(cmd));
}

#endif