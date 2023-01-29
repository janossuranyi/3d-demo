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

void JseGfxRenderer::ProcessCommandList(frameData_t* frameData)
{
	for (JseCmdEmpty* cmd = frameData->cmdHead; cmd; cmd = RCAST(JseCmdEmpty*, cmd->next)) {

		JseResult r{ JseResult::SUCCESS };

		switch (cmd->command) {
		case RC_NOP:
			break;
		case RC_BEGIN_RENDERPASS:
			core_->BeginRenderPass(((JseCmdBeginRenderpass*)cmd)->info);
			break;
		case RC_CREATE_GRAPHICS_PIPELINE:
			core_->CreateGraphicsPipeline(((JseCmdCreateGraphicsPipeline*)cmd)->info);
			break;
		case RC_VIEWPORT:
			core_->Viewport(((JseCmdViewport*)cmd)->viewport);
			break;
		case RC_SCISSOR:
			core_->Scissor(((JseCmdScissor*)cmd)->scissor);
			break;
		case RC_CREATE_SHADER: {
			std::string err;
			JseResult r;
			if ((r = core_->CreateShader(((JseCmdCreateShader*)cmd)->info, err)) != JseResult::SUCCESS) {
				Error("Shader %d error: %d - %s", ((JseCmdCreateShader*)cmd)->info.shaderId, r, err.c_str());
			}
		}
			break;
		case RC_CREATE_DESCRIPTOR_SET_LAYOUT_BINDING:
			core_->CreateDescriptorSetLayout(((JseCmdCreateDescriptorSetLayoutBindind*)cmd)->info);
			break;
		case RC_CREATE_BUFFER: {
			JseResult r;
			if ((r = core_->CreateBuffer(((JseCmdCreateBuffer*)cmd)->info)) != JseResult::SUCCESS) {
				Error("Buffer create error: %d", r);
			}
		}
			break;
		case RC_UPDATE_BUFFER: {
			JseResult r;
			if ((r = core_->UpdateBuffer(((JseCmdUpdateBuffer*)cmd)->info)) != JseResult::SUCCESS) {
				Error("Buffer update error: %d", r);
			}
		}
			break;
		case RC_BIND_VERTEX_BUFFERS:
		{
			const auto* xcmd = (JseCmdBindVertexBuffers*)cmd;
			core_->BindVertexBuffers(xcmd->firstBinding, xcmd->bindingCount, xcmd->pBuffers, xcmd->pOffsets);
		}
			break;
		case RC_BIND_GRAPHICS_PIPELINE:
			core_->BindGraphicsPipeline(((JseCmdBindGraphicsPipeline*)cmd)->pipeline);
			break;
		case RC_DRAW:
		{
			const auto* xcmd = (JseCmdDraw*)cmd;
			core_->Draw(xcmd->mode, xcmd->vertexCount, xcmd->instanceCount, xcmd->firstVertex, xcmd->firstInstance);
		}
			break;
		case RC_CREATE_IMAGE:
			core_->CreateImage(((JseCmdCreateImage*)cmd)->info);
			break;
		case RC_UPLOAD_IMAGE:
			core_->UpdateImageData(((JseCmdUploadImage*)cmd)->info);
			break;
		case RC_CREATE_DESCRIPTOR_SET:
			core_->CreateDescriptorSet(((JseCmdCreateDescriptorSet*)cmd)->info);
			break;
		case RC_WRITE_DESCRIPTOR_SET:
			core_->WriteDescriptorSet(((JseCmdWriteDescriptorSet*)cmd)->info);
			break;
		case RC_BIND_DESCRIPTOR_SETS:
		{
			const auto* xcmd = (JseCmdBindDescriptorSets*)cmd;
			core_->BindDescriptorSet(xcmd->firstSet, xcmd->descriptorSetCount, xcmd->pDescriptorSets, xcmd->dynamicOffsetCount, xcmd->pDynamicOffsets);
		}
			break;
		default:
			Error("Unhandled render command: %d", cmd->command);
		}
	}
}

void JseGfxRenderer::RenderThread()
{
	bool running{1};

	core_->BeginRendering();

	while (running) {

		JseUniqueLock lck(renderThreadMtx_);
		renderThreadSync_.wait(lck, [this] {return renderThreadDoWork_; });

		// Critical section 1
		renderThreadReady_ = false;

		lck.unlock();
		running = ! shouldTerminate_.Get();

		if (running) {
			RenderFrame(renderData_);
			core_->SwapChainNextImage();
		}

		lck.lock();
		// Critical section 2
		std::swap(renderThreadDoWork_, renderThreadReady_);

		renderThreadSync_.notify_all();
	}

	core_->EndRendering();

}

int JseGfxRenderer::RenderThreadWrapper(void* data)
{
	RCAST(JseGfxRenderer*, data)->RenderThread();

	return 0;
}

void JseGfxRenderer::ResetCommandBuffer()
{
	const uintptr_t bytesNeededForAlignment = CACHE_LINE_SIZE - ((uintptr_t)frameData_->frameMemory.get() & (CACHE_LINE_SIZE - 1));
	int size = bytesNeededForAlignment + CACHE_LINE_ALIGN(sizeof(JseCmdEmpty));

	frameData_->frameMemoryPtr.Set(size);
	JseCmdEmpty* cmd = RCAST(JseCmdEmpty*, frameData_->frameMemory.get() + bytesNeededForAlignment);
	cmd->command = RC_NOP;
	cmd->next = nullptr;
	frameData_->cmdTail = cmd;
	frameData_->cmdHead = cmd;
}


JseGfxRenderer::JseGfxRenderer()
{
	core_ = new JseGfxCoreNull();
	frameMemorySize_ = FRAME_MEM_SIZE;
	assert(CACHE_LINE_SIZE == JseGetCPUCacheLineSize());

	for (int i = 0; i < ON_FLIGHT_FRAMES; ++i) {
		frameData_ = &frames_[i];
		frameData_->frameMemory.reset(RCAST(uint8_t*, JseMemAlloc16(frameMemorySize_)), JseMemFree16);
		ResetCommandBuffer();
	}
	activeFrame_ = 0;
	renderFrame_ = 1;
	frameData_	= &frames_[activeFrame_];
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

JseGfxCore* JseGfxRenderer::GetCore()
{
	return core_;
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
#if 0
	using namespace nv_dds;

	CDDSImage image;
	image.load("d:/tokio.dds");
	image.upload_textureCubemap();
#endif
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
	if (useThread_) {
		JseUniqueLock lck(renderThreadMtx_);
		renderThreadSync_.wait(lck, [this] {return renderThreadReady_; });

		ptr = core_->GetMappedBufferPointer(id);
	}
	else {
		ptr = core_->GetMappedBufferPointer(id);
	}

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

	maxFrameMemUsage_ = std::max(maxFrameMemUsage_, end);

	for (uint32_t offset = 0; offset < bytes; offset += CACHE_LINE_SIZE) {
		//std::memset(ret + offset, 0, CPU_CACHELINE_SIZE);
		ZeroCacheLine(ret, offset);
	}

	return ret;
}
