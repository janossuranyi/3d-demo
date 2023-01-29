#include "JSE.h"
#include "JSE_GfxCoreNull.h"

#define CACHE_LINE_ALIGN(bytes) (((bytes) + CPU_CACHELINE_SIZE - 1) & ~(CPU_CACHELINE_SIZE - 1))

void JseGfxRenderer::Frame()
{
	if (!initialized_) return;

	if (!useThread_) {
		RenderFrame(frameData_);
		core_->SwapChainNextImage();
		ResetCommandBuffer();
	}
	else {
		renderSem_.wait();

		if (shouldTerminate_.Get()) {
			return;
		}

		renderFrame_ = activeFrame_;
		activeFrame_ = (activeFrame_ + 1) % ON_FLIGHT_FRAMES;

		frameData_ = &frames_[activeFrame_];
		ResetCommandBuffer();

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
			break;
		case RC_BEGIN_RENDERPASS:
			core_->BeginRenderPass(((JseBeginRenderpassCommand*)cmd)->info);
			break;
		case RC_CREATE_GRAPHICS_PIPELINE:
			core_->CreateGraphicsPipeline(((JseCreateGraphicsPipelineCommand*)cmd)->info);
			break;
		case RC_VIEWPORT:
			core_->Viewport(((JseViewportCommand*)cmd)->viewport);
			break;
		case RC_SCISSOR:
			core_->Scissor(((JseScissorCommand*)cmd)->scissor);
			break;
		case RC_CREATE_SHADER: {
			std::string err;
			JseResult r;
			if ((r = core_->CreateShader(((JseCreateShaderCommand*)cmd)->info, err)) != JseResult::SUCCESS) {
				Error("Shader %d error: %d - %s", ((JseCreateShaderCommand*)cmd)->info.shaderId, r, err.c_str());
			}
		}
			break;
		case RC_CREATE_DESCRIPTOR_SET_LAYOUT_BINDING:
			core_->CreateDescriptorSetLayout(((JseCreateDescriptorSetLayoutBinding*)cmd)->info);
			break;
		case RC_CREATE_BUFFER: {
			JseResult r;
			if ((r = core_->CreateBuffer(((JseCreateBufferCommand*)cmd)->info)) != JseResult::SUCCESS) {
				Error("Buffer create error: %d", r);
			}
		}
			break;
		case RC_UPDATE_BUFFER: {
			JseResult r;
			if ((r = core_->UpdateBuffer(((JseUpdateBufferCommand*)cmd)->info)) != JseResult::SUCCESS) {
				Error("Buffer update error: %d", r);
			}
		}
			break;
		case RC_BIND_VERTEX_BUFFERS:
		{
			const auto* xcmd = (JseBindVertexBuffersCommand*)cmd;
			core_->BindVertexBuffers(xcmd->firstBinding, xcmd->bindingCount, xcmd->pBuffers, xcmd->pOffsets);
		}
			break;
		case RC_BIND_GRAPHICS_PIPELINE:
			core_->BindGraphicsPipeline(((JseBindGraphicsPipelineCommand*)cmd)->pipeline);
			break;
		case RC_DRAW:
		{
			const auto* xcmd = (JseDrawCommand*)cmd;
			core_->Draw(xcmd->mode, xcmd->vertexCount, xcmd->instanceCount, xcmd->firstVertex, xcmd->firstInstance);
		}
			break;
		case RC_CREATE_IMAGE:
			core_->CreateImage(((JseCreateImageCommand*)cmd)->info);
			break;
		case RC_UPLOAD_IMAGE:
			core_->UpdateImageData(((JseUploadImageCommand*)cmd)->info);
			break;
		case RC_CREATE_DESCRIPTOR_SET:
			core_->CreateDescriptorSet(((JseCreateDescriptorSetCommand*)cmd)->info);
			break;
		case RC_WRITE_DESCRIPTOR_SET:
			core_->WriteDescriptorSet(((JseWriteDescriptorSetCommand*)cmd)->info);
			break;
		case RC_BIND_DESCRIPTOR_SETS:
		{
			const auto* xcmd = (JseBindDescriptorSetsCommand*)cmd;
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
		frontendSem_.wait();

		running = ! shouldTerminate_.Get();

		if (running) {
			RenderFrame(&frames_[renderFrame_]);
			core_->SwapChainNextImage();
		}

		renderSem_.post();
	}

	core_->EndRendering();

}

int JseGfxRenderer::RenderThreadWrapper(void* data)
{
	reinterpret_cast<JseGfxRenderer*>(data)->RenderThread();

	return 0;
}

void JseGfxRenderer::ResetCommandBuffer()
{
	int size = CACHE_LINE_ALIGN(sizeof(JseEmptyCommand));
	
	frameData_->frameMemoryPtr.Set(size);
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
		frameData_->frameMemory.reset(RCAST(uint8_t*, JseMemAlloc16(frameMemorySize_)), JseMemFree16);
		ResetCommandBuffer();
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
			renderThread_ = JseThread(RenderThreadWrapper, "JseRender-Thread", this);
		}
	}

	return res;
}

void* JseGfxRenderer::GetMappedBufferPointer(JseBufferID id)
{
	if (useThread_) renderSem_.wait();
	void* ptr = core_->GetMappedBufferPointer(id);
	if (useThread_) renderSem_.post();

	return ptr;
}

void JseGfxRenderer::CreateBuffer(const JseBufferCreateInfo& info, std::promise<JseResult> result)
{
	result.set_value(JseResult::SUCCESS);
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

	for (uint32_t offset = 0; offset < bytes; offset += CPU_CACHELINE_SIZE) {
		std::memset(ret + offset, 0, CPU_CACHELINE_SIZE);
	}

	return ret;
}
