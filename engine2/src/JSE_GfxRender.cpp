#include "JSE.h"
#include "JSE_GfxCoreGL46.h"

void JseGfxRenderer::Frame()
{
	renderFrame_ = activeFrame_;
	activeFrame_ = (activeFrame_ + 1) % ON_FLIGHT_FRAMES;
	frameData_ = &frames_[activeFrame_];
	frameData_->frameMemoryPtr = 0;

	JseGfxCmdEmpty* cmd = (JseGfxCmdEmpty*)R_FrameAlloc(sizeof(JseGfxCmdEmpty));
	cmd->command = RC_NOP;
	cmd->next = nullptr;
	frameData_->cmdTail = cmd;
	frameData_->cmdHead = cmd;

	frameData_t& renderData = frames_[renderFrame_];
	for (JseGfxCmdEmpty* cmd = renderData.cmdHead; cmd; cmd = (JseGfxCmdEmpty*)cmd->next) {

		JseResult r{JseResult::SUCCESS};

		switch (cmd->command) {
		case RC_NOP:
			break;
		case RC_CreateBufer:
			//r = core->CreateBuffer(((JseGfxCmdCreateBuffer*)cmd)->info);
			break;
		}
		Info("Cmd: %d, r: %d", cmd->command, r);
	}
}

JseGfxRenderer::JseGfxRenderer()
{
	core = new JseGfxCoreGL();
	frameMemorySize_ = FRAME_MEM_SIZE;
	for (int i = 0; i < ON_FLIGHT_FRAMES; ++i) {
		frameData_ = &frames_[i];
		frameData_->frameMemory.reset(reinterpret_cast<uint8_t*>(JseMemAlloc16(frameMemorySize_)), JseMemFree16);
		frameData_->frameMemoryPtr = 0;
		JseGfxCmdEmpty* cmd = (JseGfxCmdEmpty*)R_FrameAlloc(sizeof(JseGfxCmdEmpty));
		cmd->command = RC_NOP;
		cmd->next = nullptr;
		frameData_->cmdTail = cmd;
		frameData_->cmdHead = cmd;
	}
	activeFrame_ = 0;
	renderFrame_ = 1;
	frameData_ = &frames_[activeFrame_];
	CPU_CACHELINE_SIZE = JseGetCPUCacheLineSize();

}

JseGfxRenderer::~JseGfxRenderer()
{
	Info("Max frame mem usage: %d", maxFrameMemUsage_);
}

void JseGfxRenderer::CreateBuffer(const JseBufferCreateInfo& info, std::promise<JseResult> result)
{
	result.set_value(JseResult::SUCCESS);
}

uint8_t* JseGfxRenderer::R_FrameAlloc(uint32_t bytes)
{
	bytes = (bytes + CPU_CACHELINE_SIZE - 1) & ~(CPU_CACHELINE_SIZE - 1);

	uint32_t end = frameData_->frameMemoryPtr + bytes;

	if (end > frameMemorySize_) {
		throw std::runtime_error("Out of frame memory");
	}

	uint8_t* ret = frameData_->frameMemory.get() + end - bytes;

	frameData_->frameMemoryPtr += bytes;

	maxFrameMemUsage_ = std::max(maxFrameMemUsage_, frameData_->frameMemoryPtr);

	for (uint32_t offset = 0; offset < bytes; offset += CPU_CACHELINE_SIZE) {
		std::memset(ret + offset, 0, CPU_CACHELINE_SIZE);
	}
	return ret;
}
