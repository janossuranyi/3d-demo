#include "./System.h"
#include "./Heap.h"
#include "./RenderBackend.h"
#include "./RenderSystem.h"
#include "./FrameBuffer.h"
#include "./Logger.h"


#define ON_FLIGHT_FRAMES 2

#define CACHE_LINE_ALIGN(bytes) (((bytes) + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1))

namespace jsr {

	const size_t DEFAULT_FRAME_MEM_SIZE = 64 * 1024 * 1024;

	
	frameData_t				frames[ON_FLIGHT_FRAMES];
	frameData_t*			frameData;
	frameData_t*			renderData;
	std::atomic_int			maxFrameMemUsage;
	int						activeFrame;
	int						renderFrame;

	RenderSystem::RenderSystem()
	{
		view			= nullptr;
		initialized		= false;
		frameNum		= 0;
		backend			= new RenderBackend();
		vertexCache		= new VertexCache();
		programManager	= new ProgramManager();
		imageManager	= new ImageManager();
		modelManager	= new ModelManager();
		materialManager = new MaterialManager();
	}

	RenderSystem::~RenderSystem()
	{
		if (IsInitialized())
		{
			Shutdown();
		}

		Framebuffer::Shutdown();

		delete modelManager;
		delete materialManager;
		delete imageManager;
		delete programManager;
		delete vertexCache;
		delete backend;

		R_ShutdownCommandBuffers();

		initialized = false;
	}
	bool RenderSystem::Init()
	{
		if (!backend->Init())
		{
			return false;
		}

		if (!programManager->Init())	Error("[RenderSystem]: programManager init failed !");
		if (!vertexCache->Init())		Error("[RenderSystem]: vertexCache init failed !");
		if (!imageManager->Init())		Error("[RenderSystem]: imageManager init failed !");

		Framebuffer::Init();
		R_InitCommandBuffers();

		initialized = true;

		return true;
	}
	void RenderSystem::Shutdown()
	{
	}

	bool RenderSystem::IsInitialized() const
	{
		return initialized;
	}

	void RenderSystem::Frame(const emptyCommand_t* cmds)
	{

		backend->RenderCommandBuffer(cmds);
		++frameNum;
	}

	uint8_t* R_FrameAlloc(uint32_t bytes)
	{
		bytes = CACHE_LINE_ALIGN(bytes);

		uint8_t* ret{};

		const int next = frameData->frameMemoryPtr.fetch_add(bytes, std::memory_order_relaxed);
		const int end = next + bytes;

		if (end > DEFAULT_FRAME_MEM_SIZE) {
			Error("Out of frame memory");
		}

		ret = frameData->frameMemory + next;

		int x = maxFrameMemUsage.load(std::memory_order_relaxed);
		if (end > x) {
			maxFrameMemUsage.compare_exchange_weak(x, end);
		}

		for (uint32_t offset = 0; offset < bytes; offset += CACHE_LINE_SIZE) {
			//std::memset(ret + offset, 0, CACHE_LINE_SIZE);
			ZeroCacheLine(ret, offset);
		}

		return ret;
	}

	emptyCommand_t* R_GetCommandBuffer(uint32_t size)
	{
		emptyCommand_t* cmd;
		cmd = (emptyCommand_t*)R_FrameAlloc(size);
		cmd->next = nullptr;
		frameData->cmdTail->next = &cmd->command;
		frameData->cmdTail = cmd;

		return cmd;
	}

	emptyCommand_t* R_SwapCommandBuffers(bool swap)
	{
		if (swap)
		{
			renderFrame = activeFrame;
			activeFrame = (activeFrame + 1) % ON_FLIGHT_FRAMES;
			frameData = &frames[activeFrame];
		}

		R_ResetCommandBuffer();

		return frames[swap ? renderFrame : activeFrame].cmdHead;
	}

	void R_ResetCommandBuffer()
	{
		uintptr_t bytesNeededForAlignment = 0;
		const uintptr_t aligmentError = ((uintptr_t)frameData->frameMemory & (CACHE_LINE_SIZE - 1));
		if (aligmentError)
		{
			bytesNeededForAlignment = CACHE_LINE_SIZE - aligmentError;
		}

		int size = bytesNeededForAlignment + CACHE_LINE_ALIGN(sizeof(emptyCommand_t));

		frameData->frameMemoryPtr.store(size, std::memory_order_relaxed);
		emptyCommand_t* cmd = (emptyCommand_t*)frameData->frameMemory + bytesNeededForAlignment;
		std::memset(cmd, 0, sizeof(*cmd));
		cmd->command = RC_NOP;
		cmd->next = nullptr;
		frameData->cmdTail = cmd;
		frameData->cmdHead = cmd;
	}

	void R_InitCommandBuffers()
	{
		assert(CACHE_LINE_SIZE == GetCPUCacheLineSize());

		for (int i = 0; i < ON_FLIGHT_FRAMES; ++i) {
			frameData = &frames[i];
			frameData->frameMemory = (uint8_t*)MemAlloc16(DEFAULT_FRAME_MEM_SIZE);
			R_ResetCommandBuffer();
		}
		activeFrame = 0;
		renderFrame = 1;
		frameData = &frames[activeFrame];
		renderData = &frames[renderFrame];

	}

	void R_ShutdownCommandBuffers()
	{
		for (int i = 0; i < ON_FLIGHT_FRAMES; ++i)
		{
			MemFree16(frames[i].frameMemory);
		}
	}


	RenderSystem renderSystem;

}
