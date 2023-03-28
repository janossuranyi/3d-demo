#include "JSE.h"
#include "JSE_GfxCoreNull.h"

#define CACHE_LINE_ALIGN(bytes) (((bytes) + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1))
#define RUN_CORE_CMD(stmt) do {\
if (useThread_) {\
	WaitForRenderFinish();\
	core_->BeginRendering();\
	stmt;\
	core_->EndRendering();\
} else {\
	stmt;\
}\
} while(0)

namespace js
{
	void GfxRenderer::Frame(bool swapBuffers)
	{
		if (!initialized_) return;

		if (!useThread_) {
			RenderFrame(frameData_);
			if (swapBuffers) core_->SwapChainNextImage();
			ResetCommandBuffer();
		}
		else
		{
			//WaitForRenderFinish();
			std::unique_lock<std::mutex> lck(renderThreadMtx_);
			renderThreadSync_.wait(lck, [this] {return renderThreadReady_; });

			if (shouldTerminate_.load(std::memory_order_relaxed)) {
				return;
			}

			renderFrame_ = activeFrame_;
			activeFrame_ = (activeFrame_ + 1) % ON_FLIGHT_FRAMES;

			frameData_ = &frames_[activeFrame_];
			renderData_ = &frames_[renderFrame_];
			ResetCommandBuffer();

			renderThreadDoWork_ = true;
			renderThreadReady_ = false;
			renderThreadSwapBuffers_.store(swapBuffers, std::memory_order_relaxed);
			renderThreadSync_.notify_all();
		}
	}

	void GfxRenderer::RenderFrame(frameData_t* renderData)
	{
		ProcessCommandList(renderData);
	}

	void GfxRenderer::operator()(const JseCmdEmpty& cmd)
	{
	}

	void GfxRenderer::operator()(const JseCmdBeginRenderpass& cmd) {
		lastResult_ = core_->BeginRenderPass(cmd.info);
	}

	void GfxRenderer::operator()(const JseCmdCreateGraphicsPipeline& cmd)
	{
		lastResult_ = core_->CreateGraphicsPipeline(cmd.info);
	}

	void GfxRenderer::operator()(const JseCmdViewport& cmd)
	{
		core_->Viewport(cmd.viewport);
	}

	void GfxRenderer::operator()(const JseCmdScissor& cmd)
	{
		core_->Scissor(cmd.scissor);
	}

	void GfxRenderer::operator()(const JseCmdCreateShader& cmd)
	{
		std::string err;
		Result r{};
		if ((r = core_->GenShader(cmd.info, err)) != Result::SUCCESS) {
			Error("Shader %d error: %d - %s", cmd.info.shaderId, r, err.c_str());
		}

		lastResult_ = r;
	}

	void GfxRenderer::operator()(const JseCmdCreateDescriptorSetLayoutBindind& cmd)
	{
		lastResult_ = core_->CreateDescriptorSetLayout(cmd.info);
	}

	void GfxRenderer::operator()(const JseCmdCreateBuffer& cmd)
	{
		Result r;
		if ((r = core_->CreateBuffer(cmd.info)) != Result::SUCCESS) {
			Error("Buffer create error: %d", r);
		}
		lastResult_ = r;
	}

	void GfxRenderer::operator()(const JseCmdUpdateBuffer& cmd)
	{
		Result r;
		if ((r = core_->UpdateBuffer(cmd.info)) != Result::SUCCESS) {
			Error("Buffer update error: %d", r);
		}
		lastResult_ = r;
	}

	void GfxRenderer::operator()(const JseCmdDeleteBuffer& cmd)
	{
		core_->DestroyBuffer(cmd.buffer);
	}

	void GfxRenderer::operator()(const JseCmdBindVertexBuffers& cmd)
	{
		core_->BindVertexBuffers(cmd.firstBinding, cmd.bindingCount, cmd.pBuffers, cmd.pOffsets);
	}

	void GfxRenderer::operator()(const JseCmdBindGraphicsPipeline& cmd)
	{
		lastResult_ = core_->BindGraphicsPipeline(cmd.pipeline);
	}

	void GfxRenderer::operator()(const JseCmdDraw& cmd)
	{
		core_->Draw(cmd.mode, cmd.vertexCount, cmd.instanceCount, cmd.firstVertex, cmd.firstInstance);
	}

	void GfxRenderer::operator()(const JseCmdDrawIndexed& cmd)
	{
		core_->DrawIndexed(cmd.mode, cmd.indexCount, cmd.instanceCount, cmd.firstIndex, cmd.vertexOffset, cmd.firstInstance);
	}

	void GfxRenderer::operator()(const JseCmdCreateImage& cmd)
	{
		lastResult_ = core_->CreateImage(cmd.info);
	}

	void GfxRenderer::operator()(const JseCmdUploadImage& cmd)
	{
		lastResult_ = core_->UpdateImageData(cmd.info);
	}

	void GfxRenderer::operator()(const JseCmdCreateDescriptorSet& cmd)
	{
		lastResult_ = core_->CreateDescriptorSet(cmd.info);
	}

	void GfxRenderer::operator()(const JseCmdWriteDescriptorSet& cmd)
	{
		lastResult_ = core_->WriteDescriptorSet(cmd.info);
	}

	void GfxRenderer::operator()(const JseCmdBindDescriptorSets& cmd)
	{
		lastResult_ = core_->BindDescriptorSet(cmd.firstSet, cmd.descriptorSetCount, cmd.pDescriptorSets, cmd.dynamicOffsetCount, cmd.pDynamicOffsets);
	}

	void GfxRenderer::operator()(const JseCreateFenceCmd& cmd)
	{
		if (core_->CreateFence(cmd.id) != Result::SUCCESS) {
			Error("CreateFence failed");
		}
	}

	void GfxRenderer::operator()(const JseDeleteFenceCmd& cmd)
	{
		if (core_->DeleteFence(cmd.id) != Result::SUCCESS) {
			Error("DeleteFence failed");
		}
	}

	void GfxRenderer::operator()(const JseWaitSyncCmd& cmd)
	{
		if (core_->WaitSync(cmd.id, cmd.timeout) != Result::SUCCESS) {
			Error("WaitSync failed");
		}
	}

	void GfxRenderer::ProcessCommandList(frameData_t* frameData)
	{
		for (JseCmdWrapper* cmd = frameData->cmdHead; cmd; cmd = cmd->next) {

			std::visit(*this, cmd->command);
		}
	}

	void GfxRenderer::RenderThread()
	{
		bool running{ 1 };


		while (running) {

			std::unique_lock<std::mutex> lck(renderThreadMtx_);
			renderThreadSync_.wait(lck, [this] {return renderThreadDoWork_; });

			// Critical section 1
			renderThreadReady_ = false;

			lck.unlock();
			running = !shouldTerminate_.load(std::memory_order_relaxed);

			if (running) {
				core_->BeginRendering();
				{
					RenderFrame(renderData_);
					//core_->FlushCommandBuffers();
					if (renderThreadSwapBuffers_.load(std::memory_order_relaxed)) core_->SwapChainNextImage();
				}
				core_->EndRendering();
			}

			lck.lock();
			// Critical section 2
			std::swap(renderThreadDoWork_, renderThreadReady_);

			renderThreadSync_.notify_all();
		}
	}

	int GfxRenderer::RenderThreadWrapper(void* data)
	{
		reinterpret_cast<GfxRenderer*>(data)->RenderThread();

		return 0;
	}

	void GfxRenderer::ResetCommandBuffer()
	{
		const uintptr_t bytesNeededForAlignment = CACHE_LINE_SIZE - ((uintptr_t)frameData_->frameMemory.get() & (CACHE_LINE_SIZE - 1));
		int size = bytesNeededForAlignment + CACHE_LINE_ALIGN(sizeof(JseCmdWrapper));

		frameData_->frameMemoryPtr.store(size, std::memory_order_relaxed);
		JseCmdWrapper* cmd = RCAST(JseCmdWrapper*, frameData_->frameMemory.get() + bytesNeededForAlignment);
		cmd->command = JseCmdEmpty{};
		cmd->next = nullptr;
		frameData_->cmdTail = cmd;
		frameData_->cmdHead = cmd;
	}


	GfxRenderer::GfxRenderer(GfxCore* core) : GfxRenderer(core, DEFAULT_FRAME_MEM_SIZE) {}
	GfxRenderer::GfxRenderer(GfxCore* core, int frameMemorySize)
	{
		core_ = core;
		frameMemorySize_ = frameMemorySize;
		assert(CACHE_LINE_SIZE == JseGetCPUCacheLineSize());

		for (int i = 0; i < ON_FLIGHT_FRAMES; ++i) {
			frameData_ = &frames_[i];
			frameData_->frameMemory.reset(static_cast<uint8_t*>(JseMemAlloc16(frameMemorySize_)), JseMemFree16);
			ResetCommandBuffer();
		}
		activeFrame_ = 0;
		renderFrame_ = 1;
		frameData_ = &frames_[activeFrame_];
		renderData_ = &frames_[renderFrame_];
	}

	GfxRenderer::~GfxRenderer()
	{
		Info("Max frame mem usage: %d", maxFrameMemUsage_.load());
		if (useThread_)
		{
			{
				std::unique_lock<std::mutex> lck(renderThreadMtx_);
				renderThreadSync_.wait(lck, [this] {return renderThreadReady_; });
				shouldTerminate_.store(true, std::memory_order_relaxed);
				renderThreadDoWork_ = true;
			}
			renderThreadSync_.notify_all();
			renderThread_.join();
			useThread_ = false;
		}

		core_->BeginRendering();
		/* final GPU works */
		Frame(false);

		core_->Shutdown();
	}

	uint32_t GfxRenderer::NextID()
	{
		return SCAST(uint32_t, nextId_.fetch_add(1));
	}

	JseShaderID GfxRenderer::GenShader()
	{
		return shaderGenerator_.next();
	}

	JseImageID GfxRenderer::GenImage()
	{
		return imageGenerator_.next();
	}

	JseBufferID GfxRenderer::GenBuffer()
	{
		return bufferGenerator_.next();
	}

	GfxCore* GfxRenderer::core()
	{
		return core_;
	}

	void GfxRenderer::Invoke(Invokable func)
	{
		if (!func) return;

		if (useThread_) {
			std::unique_lock<std::mutex> lck(renderThreadMtx_);
			renderThreadSync_.wait(lck, [this] {return renderThreadReady_; });
			core_->BeginRendering();
			{
				func();
			}
			core_->EndRendering();
		}
		else {
			func();
		}
	}

	Result GfxRenderer::CreateImage(const JseImageCreateInfo& x)
	{
		Result r;
		RUN_CORE_CMD(r = core_->CreateImage(x));

		return r;
	}

	Result GfxRenderer::UploadImage(const JseImageUploadInfo& x)
	{
		Result r;
		RUN_CORE_CMD(r = core_->UpdateImageData(x));

		return r;
	}

	Result GfxRenderer::InitCore(int w, int h, bool fs, bool useThread)
	{
		JseSurfaceCreateInfo sci{};
		sci.alphaBits = 8;
		sci.colorBits = 24;
		sci.depthBits = 24;
		sci.fullScreen = fs;
		sci.height = h;
		sci.stencilBits = 8;
		sci.swapInterval = 1;
		sci.width = w;
		sci.srgb = true;

		auto res = core_->CreateSurface(sci);

		if (res == Result::SUCCESS) {
			initialized_ = true;
			useThread_ = useThread;

			if (useThread_) {
				core_->EndRendering();
				renderThreadDoWork_ = false;
				renderThreadReady_ = true;
				renderThread_ = Thread(RenderThreadWrapper, "JseRender-Thread", this);
			}
		}
		
		return res;
	}

	void* GfxRenderer::GetMappedBufferPointer(JseBufferID id)
	{
		void* ptr;
		RUN_CORE_CMD(ptr = core_->GetMappedBufferPointer(id));

		return ptr;
	}

	uint8_t* GfxRenderer::R_FrameAlloc(uint32_t bytes)
	{
		bytes = CACHE_LINE_ALIGN(bytes);

		int	end{};
		uint8_t* ret{};

		end = frameData_->frameMemoryPtr.fetch_add(bytes, std::memory_order_relaxed) + bytes;

		if (end > frameMemorySize_) {
			throw std::runtime_error("Out of frame memory");
		}

		ret = frameData_->frameMemory.get() + end - bytes;

		int x = maxFrameMemUsage_.load(std::memory_order_relaxed);
		if (end > x) {
			maxFrameMemUsage_.compare_exchange_weak(x, end);
		}

		for (uint32_t offset = 0; offset < bytes; offset += CACHE_LINE_SIZE) {
			//std::memset(ret + offset, 0, CPU_CACHELINE_SIZE);
			ZeroCacheLine(ret, offset);
		}

		return ret;
	}

	JseCmdWrapper* GfxRenderer::GetCommandBuffer()
	{
		JseCmdWrapper* cmd;
		cmd = new (R_FrameAlloc(sizeof(*cmd))) JseCmdWrapper();
		cmd->next = nullptr;
		frameData_->cmdTail->next = cmd;
		frameData_->cmdTail = cmd;

		return cmd;
	}

	void GfxRenderer::SetVSyncInterval(int x) {
		RUN_CORE_CMD(core_->SetVSyncInterval(x));
	}

	void GfxRenderer::WaitForRenderFinish()
	{
		if (useThread_) {
			std::unique_lock<std::mutex> lck(renderThreadMtx_);
			renderThreadSync_.wait(lck, [this] {return renderThreadReady_; });
		}
	}

	JsType GfxRenderer::typeIndex() const
	{
		return std::type_index(typeid(GfxRenderer));
	}

	Result GfxRenderer::WaitSync(JseFenceID id, uint64_t timeout) {
		Result r;
		RUN_CORE_CMD(r = core_->WaitSync(id, timeout));

		return r;
	}
}