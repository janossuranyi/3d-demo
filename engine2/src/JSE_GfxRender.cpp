#include "JSE.h"
#include "JSE_GfxCoreNull.h"

#define CACHE_LINE_ALIGN(bytes) (((bytes) + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1))

void JseGfxRenderer::Frame()
{
	if (!initialized_) return;

	if (!useThread_) {
		RenderFrame(frameData_);
		core_->SwapChainNextImage();
		ResetCommandBuffer();
	}
	else {
		
		JseUniqueLock lck(renderThreadMtx_);
		renderThreadSync_.wait(lck, [this] {return renderThreadReady_; });

		if (shouldTerminate_.Get()) {
			return;
		}

		renderFrame_ = activeFrame_;
		activeFrame_ = (activeFrame_ + 1) % ON_FLIGHT_FRAMES;

		frameData_ = &frames_[activeFrame_];
		renderData_ = &frames_[renderFrame_];
		ResetCommandBuffer();

		renderThreadDoWork_ = true;
		renderThreadReady_ = false;
		renderThreadSync_.notify_all();
	}
}

void JseGfxRenderer::RenderFrame(frameData_t* renderData)
{
	ProcessCommandList(renderData);
}

void JseGfxRenderer::operator()(const JseCmdEmpty& cmd)
{
}

void JseGfxRenderer::operator()(const JseCmdBeginRenderpass& cmd) {
	lastResult_ = core_->BeginRenderPass(cmd.info);
}

void JseGfxRenderer::operator()(const JseCmdCreateGraphicsPipeline& cmd)
{
	lastResult_ = core_->CreateGraphicsPipeline(cmd.info);
}

void JseGfxRenderer::operator()(const JseCmdViewport& cmd)
{
	core_->Viewport(cmd.viewport);
}

void JseGfxRenderer::operator()(const JseCmdScissor& cmd)
{
	core_->Scissor(cmd.scissor);
}

void JseGfxRenderer::operator()(const JseCmdCreateShader& cmd)
{
	std::string err;
	JseResult r{};
	if ((r = core_->CreateShader(cmd.info, err)) != JseResult::SUCCESS) {
		Error("Shader %d error: %d - %s", cmd.info.shaderId, r, err.c_str());
	}

	lastResult_ = r;
}

void JseGfxRenderer::operator()(const JseCmdCreateDescriptorSetLayoutBindind& cmd)
{
	lastResult_ = core_->CreateDescriptorSetLayout(cmd.info);
}

void JseGfxRenderer::operator()(const JseCmdCreateBuffer& cmd)
{
	JseResult r;
	if ((r = core_->CreateBuffer(cmd.info)) != JseResult::SUCCESS) {
		Error("Buffer create error: %d", r);
	}
	lastResult_ = r;
}

void JseGfxRenderer::operator()(const JseCmdUpdateBuffer& cmd)
{
	JseResult r;
	if ((r = core_->UpdateBuffer(cmd.info)) != JseResult::SUCCESS) {
		Error("Buffer update error: %d", r);
	}
	lastResult_ = r;
}

void JseGfxRenderer::operator()(const JseCmdBindVertexBuffers& cmd)
{
	core_->BindVertexBuffers(cmd.firstBinding, cmd.bindingCount, cmd.pBuffers, cmd.pOffsets);
}

void JseGfxRenderer::operator()(const JseCmdBindGraphicsPipeline& cmd)
{
	lastResult_ = core_->BindGraphicsPipeline(cmd.pipeline);
}

void JseGfxRenderer::operator()(const JseCmdDraw& cmd)
{
	core_->Draw(cmd.mode, cmd.vertexCount, cmd.instanceCount, cmd.firstVertex, cmd.firstInstance);
}

void JseGfxRenderer::operator()(const JseCmdDrawIndexed& cmd)
{
	core_->DrawIndexed(cmd.mode, cmd.indexCount, cmd.instanceCount, cmd.firstIndex, cmd.vertexOffset, cmd.firstInstance);
}

void JseGfxRenderer::operator()(const JseCmdCreateImage& cmd)
{
	lastResult_ = core_->CreateImage(cmd.info);
}

void JseGfxRenderer::operator()(const JseCmdUploadImage& cmd)
{
	lastResult_ = core_->UpdateImageData(cmd.info);
}

void JseGfxRenderer::operator()(const JseCmdCreateDescriptorSet& cmd)
{
	lastResult_ = core_->CreateDescriptorSet(cmd.info);
}

void JseGfxRenderer::operator()(const JseCmdWriteDescriptorSet& cmd)
{
	lastResult_ = core_->WriteDescriptorSet(cmd.info);
}

void JseGfxRenderer::operator()(const JseCmdBindDescriptorSets& cmd)
{
	lastResult_ = core_->BindDescriptorSet(cmd.firstSet, cmd.descriptorSetCount, cmd.pDescriptorSets, cmd.dynamicOffsetCount, cmd.pDynamicOffsets);
}

void JseGfxRenderer::operator()(const JseCreateFenceCmd& cmd)
{
	if (core_->CreateFence(cmd.id) != JseResult::SUCCESS) {
		Error("CreateFence failed");
	}
}

void JseGfxRenderer::operator()(const JseDeleteFenceCmd& cmd)
{
	if (core_->DeleteFence(cmd.id) != JseResult::SUCCESS) {
		Error("DeleteFence failed");
	}
}

void JseGfxRenderer::operator()(const JseWaitSyncCmd& cmd)
{
	if (core_->WaitSync(cmd.id, cmd.timeout) != JseResult::SUCCESS) {
		Error("WaitSync failed");
	}
}

void JseGfxRenderer::ProcessCommandList(frameData_t* frameData)
{
	for (JseCmdWrapper* cmd = frameData->cmdHead; cmd; cmd = cmd->next) {

		std::visit(*this, cmd->command);
	}
}

void JseGfxRenderer::RenderThread()
{
	bool running{1};


	while (running) {

		JseUniqueLock lck(renderThreadMtx_);
		renderThreadSync_.wait(lck, [this] {return renderThreadDoWork_; });

		// Critical section 1
		renderThreadReady_ = false;

		lck.unlock();
		running = ! shouldTerminate_.Get();

		if (running) {
			core_->BeginRendering();
			{
				RenderFrame(renderData_);
				core_->SwapChainNextImage();
			}
			core_->EndRendering();
		}

		lck.lock();
		// Critical section 2
		std::swap(renderThreadDoWork_, renderThreadReady_);

		renderThreadSync_.notify_all();
	}
}

int JseGfxRenderer::RenderThreadWrapper(void* data)
{
	RCAST(JseGfxRenderer*, data)->RenderThread();

	return 0;
}

void JseGfxRenderer::ResetCommandBuffer()
{
	const uintptr_t bytesNeededForAlignment = CACHE_LINE_SIZE - ((uintptr_t)frameData_->frameMemory.get() & (CACHE_LINE_SIZE - 1));
	int size = bytesNeededForAlignment + CACHE_LINE_ALIGN(sizeof(JseCmdWrapper));

	frameData_->frameMemoryPtr.Set(size);
	JseCmdWrapper* cmd = RCAST(JseCmdWrapper*, frameData_->frameMemory.get() + bytesNeededForAlignment);
	cmd->command = JseCmdEmpty{};
	cmd->next = nullptr;
	frameData_->cmdTail = cmd;
	frameData_->cmdHead = cmd;
}


JseGfxRenderer::JseGfxRenderer() : JseGfxRenderer(DEFAULT_FRAME_MEM_SIZE)
{
}

JseGfxRenderer::JseGfxRenderer(int frameMemorySize)
{
	core_ = std::make_shared<JseGfxCoreNull>();
	frameMemorySize_ = frameMemorySize;
	assert(CACHE_LINE_SIZE == JseGetCPUCacheLineSize());

	for (int i = 0; i < ON_FLIGHT_FRAMES; ++i) {
		frameData_ = &frames_[i];
		frameData_->frameMemory.reset(RCAST(uint8_t*, JseMemAlloc16(frameMemorySize_)), JseMemFree16);
		ResetCommandBuffer();
	}
	activeFrame_ = 0;
	renderFrame_ = 1;
	frameData_ = &frames_[activeFrame_];
	renderData_ = &frames_[renderFrame_];
}

JseGfxRenderer::~JseGfxRenderer()
{
	Info("Max frame mem usage: %d", maxFrameMemUsage_);
	if (useThread_) {
		{
			JseUniqueLock lck(renderThreadMtx_);
			renderThreadSync_.wait(lck, [this] {return renderThreadReady_; });
			shouldTerminate_.Set(true);
			renderThreadDoWork_ = true;
		}
		renderThreadSync_.notify_all();
		renderThread_.join();
	}
	core_->Shutdown();
}

uint32_t JseGfxRenderer::NextID()
{
	return SCAST(uint32_t, nextId_.Add(1));
}

JseGfxCore* JseGfxRenderer::GetCore()
{
	return core_.get();
}

void JseGfxRenderer::Invoke(Invokable func)
{
	if (!func) return;

	if (useThread_) {
		JseUniqueLock lck(renderThreadMtx_);
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

JseResult JseGfxRenderer::CreateImage(const JseImageCreateInfo& x)
{
	JseResult r{};

	Invoke([this, &r, x] {r = core_->CreateImage(x); });

	return r;
}

JseResult JseGfxRenderer::UploadImage(const JseImageUploadInfo& x)
{
	JseResult r{};

	Invoke([this, &r, x] {r = core_->UpdateImageData(x); });

	return r;
}

void JseGfxRenderer::SetCore(JseSharedPtr<JseGfxCore> core)
{
	if (initialized_) {
		throw std::runtime_error("Cannot change the Core after initialization!!!");
	}
	core_ = core;
}

JseResult JseGfxRenderer::InitCore(int w, int h, bool fs, bool useThread)
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

	if (res == JseResult::SUCCESS) {
		initialized_ = true;
		useThread_ = useThread;

		if (useThread_) {
			core_->EndRendering();
			renderThreadDoWork_ = false;
			renderThreadReady_ = true;
			renderThread_ = JseThread(RenderThreadWrapper, "JseRender-Thread", this);
		}
	}

	return res;
}

void* JseGfxRenderer::GetMappedBufferPointer(JseBufferID id)
{
	void* ptr{};

	Invoke([this,&ptr,id] {ptr = core_->GetMappedBufferPointer(id); });

	return ptr;
}

uint8_t* JseGfxRenderer::R_FrameAlloc(uint32_t bytes)
{
	bytes = CACHE_LINE_ALIGN(bytes);

	int			end{};
	uint8_t*	ret{};

	end = frameData_->frameMemoryPtr.Add(bytes) + bytes;

	if (end > frameMemorySize_) {
		throw std::runtime_error("Out of frame memory");
	}

	ret = frameData_->frameMemory.get() + end - bytes;

	if (end > maxFrameMemUsage_) {
		maxFrameMemUsage_ = end;
	}

	for (uint32_t offset = 0; offset < bytes; offset += CACHE_LINE_SIZE) {
		//std::memset(ret + offset, 0, CPU_CACHELINE_SIZE);
		ZeroCacheLine(ret, offset);
	}

	return ret;
}

JseCmdWrapper* JseGfxRenderer::GetCommandBuffer()
{
	JseCmdWrapper* cmd;
	cmd = new (R_FrameAlloc(sizeof(*cmd))) JseCmdWrapper();
	cmd->next = nullptr;
	frameData_->cmdTail->next = cmd;
	frameData_->cmdTail = cmd;

	return cmd;
}

void JseGfxRenderer::SetVSyncInterval(int x) {
	Invoke([this,x] {core_->SetVSyncInterval(x); });
}

void JseGfxRenderer::WaitForGpuReady()
{
	if (useThread_) {
		JseUniqueLock lck(renderThreadMtx_);
		renderThreadSync_.wait(lck, [this] {return renderThreadReady_; });
	}
}

JseType JseGfxRenderer::typeIndex() const
{
	return std::type_index(typeid(JseGfxRenderer));
}

JseResult JseGfxRenderer::WaitSync(JseFenceID id, uint64_t timeout) {
	JseResult r{};
	Invoke([id, timeout, this, &r] {r = core_->WaitSync(id, timeout); });

	return r;
}