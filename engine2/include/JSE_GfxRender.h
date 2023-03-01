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
struct JseCmdDrawIndexed {
	JseTopology mode;
	uint32_t indexCount;
	uint32_t instanceCount;
	uint32_t vertexOffset;
	uint32_t firstIndex;
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
struct JseCmdDeleteBuffer {
	JseBufferID buffer;
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
	JseCmdDrawIndexed,
	JseCmdBindVertexBuffers,
	JseCmdBindGraphicsPipeline,
	JseCmdUpdateBuffer,
	JseCmdCreateBuffer,
	JseCmdDeleteBuffer,
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

namespace js
{
	class GfxRenderer : public Module {
	public:
		static const size_t DEFAULT_FRAME_MEM_SIZE = 64 * 1024 * 1024;
		static const size_t ON_FLIGHT_FRAMES = 2;
	private:
		struct frameData_t {
			std::atomic_int				frameMemoryPtr;
			JsSharedPtr<uint8_t>		frameMemory;
			JseCmdWrapper* cmdTail;
			JseCmdWrapper* cmdHead;
		};

		JsSharedPtr<GfxCore> core_;
		js::Thread				renderThread_;
		std::mutex				renderThreadMtx_;
		std::condition_variable	renderThreadSync_;
		bool					renderThreadReady_;
		bool					renderThreadDoWork_;
		std::atomic_int			shouldTerminate_{ 0 };
		std::atomic_int			nextId_{ 1 };
		std::atomic_bool		renderThreadSwapBuffers_{ true };
		size_t					frameMemorySize_{ 0 };
		int						maxFrameMemUsage_{ 0 };
		frameData_t				frames_[ON_FLIGHT_FRAMES];
		frameData_t* frameData_;
		frameData_t* renderData_;
		int						activeFrame_;
		int						renderFrame_;
		bool					useThread_{};
		bool					threadRunning_{};
		bool					initialized_{};

		void ResetCommandBuffer();

		Result lastResult_;
		JseHandleGenerator<JseShaderID> shaderGenerator_;
		JseHandleGenerator<JseImageID> imageGenerator_;
		JseHandleGenerator<JseBufferID> bufferGenerator_;
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
		void operator()(const JseCmdDeleteBuffer& cmd);
		void operator()(const JseCmdBindVertexBuffers& cmd);
		void operator()(const JseCmdBindGraphicsPipeline& cmd);
		void operator()(const JseCmdDraw& cmd);
		void operator()(const JseCmdDrawIndexed& cmd);
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

		GfxRenderer();
		GfxRenderer(int frameMemorySize);
		~GfxRenderer();

		JsType typeIndex() const override;

		uint32_t NextID();

		JseShaderID CreateShader();
		JseImageID	CreateImage();
		JseBufferID	CreateBuffer();

		JsSharedPtr<GfxCore> core();
		js::Result		CreateImage(const JseImageCreateInfo& x);
		js::Result		UploadImage(const JseImageUploadInfo& x);
		js::Result		InitCore(int w, int h, bool fs, bool useThread);

		uint8_t* R_FrameAlloc(uint32_t bytes);
		JseCmdWrapper* GetCommandBuffer();

		void	Invoke(Invokable func);
		void*	GetMappedBufferPointer(JseBufferID id);
		void	SubmitCommand(const JseCmd& cmd);
		void	SetCore(JsSharedPtr<GfxCore> core);
		void	Frame(bool swapBuffers = true);
		void	RenderFrame(frameData_t* renderData);
		void	ProcessCommandList(frameData_t* frameData);
		void	RenderThread();
		void	SetVSyncInterval(int x);
		void	WaitForRenderThreadReady();
		Result	WaitSync(JseFenceID id, uint64_t timeout);

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

	inline 	void GfxRenderer::SubmitCommand(const JseCmd& cmd) {
		auto* p = GetCommandBuffer();
		std::memcpy(&p->command, &cmd, sizeof(cmd));
	}
}
#endif