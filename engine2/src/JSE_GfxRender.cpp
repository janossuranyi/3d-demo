#include "JSE.h"
#include "JSE_GfxCoreNull.h"


void JseGfxRenderer::Frame()
{
	if (!initialized_) return;

	if (!useThread_) {
		RenderFrame(frameData_);
		ResetCommandBuffer();
	}
	else {
		renderSem_.wait();

		if (shouldTerminate_.Get()) {
			return;
		}

		renderFrame_ = activeFrame_;
		activeFrame_ = (activeFrame_ + 1) % ON_FLIGHT_FRAMES;

		SDL_AtomicLock(&frameSwitchLck_);
		{
			frameData_ = &frames_[activeFrame_];
			ResetCommandBuffer();
		}
		SDL_AtomicUnlock(&frameSwitchLck_);

		frontendSem_.post();
	}
}

void JseGfxRenderer::RenderFrame(frameData_t* renderData)
{
	ProcessCommandList(renderData);
}

void JseGfxRenderer::ProcessCommandList(frameData_t* frameData)
{
	for (JseEmptyCommand* cmd = frameData->cmdHead; cmd; cmd = RCAST(JseEmptyCommand*, cmd->next)) {

		JseResult r{ JseResult::SUCCESS };

		switch (cmd->command) {
		case RC_NOP:
			Info("RC_NOP");
			break;
		default:
			Error("Unhandled render command: %d", cmd->command);
		}
	}
}

void JseGfxRenderer::RenderThread()
{
	bool running{1};

	while (running) {
		frontendSem_.wait();

		running = ! shouldTerminate_.Get();

		if (running) {
			RenderFrame(&frames_[renderFrame_]);
		}

		renderSem_.post();
	}
}

int JseGfxRenderer::RenderThreadWrapper(void* data)
{
	reinterpret_cast<JseGfxRenderer*>(data)->RenderThread();

	return 0;
}

void JseGfxRenderer::ResetCommandBuffer()
{
	frameData_->frameMemoryPtr.Set(sizeof(JseEmptyCommand));
	JseEmptyCommand* cmd = RCAST(JseEmptyCommand*, frameData_->frameMemory.get());
	cmd->command = RC_NOP;
	cmd->next = nullptr;
	frameData_->cmdTail = cmd;
	frameData_->cmdHead = cmd;
}

JseGfxRenderer::JseGfxRenderer()
{
	core_ = new JseGfxCoreNull();
	frameMemorySize_ = FRAME_MEM_SIZE;
	CPU_CACHELINE_SIZE = JseGetCPUCacheLineSize();
	for (int i = 0; i < ON_FLIGHT_FRAMES; ++i) {
		frameData_ = &frames_[i];
		frameData_->frameMemory.reset(reinterpret_cast<uint8_t*>(JseMemAlloc16(frameMemorySize_)), JseMemFree16);
		frameData_->frameMemoryPtr.Set(0);
		JseEmptyCommand* cmd = RCAST(JseEmptyCommand*, R_FrameAlloc(sizeof(JseEmptyCommand)));
		cmd->command = RC_NOP;
		cmd->next = nullptr;
		frameData_->cmdTail = cmd;
		frameData_->cmdHead = cmd;
	}
	activeFrame_ = 0;
	renderFrame_ = 1;
	frameData_ = &frames_[activeFrame_];

}

JseGfxRenderer::~JseGfxRenderer()
{
	Info("Max frame mem usage: %d", maxFrameMemUsage_);
	if (useThread_) {
		shouldTerminate_ = true;
		frontendSem_.post();
		renderThread_.join();
	}
}

void JseGfxRenderer::SetCore(JseGfxCore* core)
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
	
	auto res = core_->CreateSurface(sci);
	if (res == JseResult::SUCCESS) {
		initialized_ = true;
		useThread_ = useThread;

		if (useThread_) {
			renderThread_ = JseThread(RenderThreadWrapper, "JseRenderer-Thread", this);
		}
	}

	return res;
}

void JseGfxRenderer::CreateBuffer(const JseBufferCreateInfo& info, std::promise<JseResult> result)
{
	result.set_value(JseResult::SUCCESS);
}

uint8_t* JseGfxRenderer::R_FrameAlloc(uint32_t bytes)
{
	bytes = (bytes + CPU_CACHELINE_SIZE - 1) & ~(CPU_CACHELINE_SIZE - 1);

	int			end{};
	uint8_t*	ret{};

	end = frameData_->frameMemoryPtr.Add(bytes) + bytes;

	if (end > frameMemorySize_) {
		throw std::runtime_error("Out of frame memory");
	}

	ret = frameData_->frameMemory.get() + end - bytes;

	maxFrameMemUsage_ = std::max(maxFrameMemUsage_, end);

	for (uint32_t offset = 0; offset < bytes; offset += CPU_CACHELINE_SIZE) {
		std::memset(ret + offset, 0, CPU_CACHELINE_SIZE);
	}

	return ret;
}
