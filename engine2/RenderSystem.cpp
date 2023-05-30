#include <imgui.h>
#include "./System.h"
#include "./Heap.h"
#include "./RenderBackend.h"
#include "./RenderSystem.h"
#include "./FrameBuffer.h"
#include "./Logger.h"


#define ON_FLIGHT_FRAMES 2

#define CACHE_LINE_ALIGN(bytes) (((bytes) + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1))
#define ALIGN(a) (((a) + 15) & 15)

namespace jsr {

	const size_t DEFAULT_FRAME_MEM_SIZE = 16 * 1024 * 1024;

	renderGlobals_t			renderGlobals{};
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
		defaultMaterial = {};
	}

	RenderSystem::~RenderSystem()
	{
		if (IsInitialized())
		{
			Shutdown();

			Framebuffer::Shutdown();

			delete modelManager;
			delete materialManager;
			delete imageManager;
			delete programManager;
			delete vertexCache;
			delete backend;
			delete unitRectTris;

			ImGui::DestroyContext();

			R_ShutdownCommandBuffers();

			initialized = false;
			Info("Max frame memory used: %d", maxFrameMemUsage.load());
		}
	}
	bool RenderSystem::Init()
	{
		ImGui::CreateContext();

		if (!backend->Init())
		{
			return false;
		}

		if (!vertexCache->Init())		Error("[RenderSystem]: vertexCache init failed !");
		if (!programManager->Init())	Error("[RenderSystem]: programManager init failed !");
		if (!imageManager->Init())		Error("[RenderSystem]: imageManager init failed !");

		Framebuffer::Init();
		R_InitCommandBuffers();

		unitRectTris = R_CreateFullScreenRect();

		defaultMaterial = materialManager->CreateMaterial("_defaultMaterial");
		stage_t& s = defaultMaterial->GetStage(STAGE_DEBUG);
		s.alphaCutoff = 0.5f;
		s.coverage = COVERAGE_SOLID;
		s.cullMode = CULL_NONE;
		s.diffuseScale = glm::vec4(0.1f,0.00f,0.1f,1.0f);
		s.roughnessScale = 0.4f;
		s.metallicScale = 0.0f;
		s.enabled = true;
		s.SetImage(IMU_DIFFUSE, imageManager->globalImages.whiteImage);
		s.SetImage(IMU_AORM, imageManager->globalImages.whiteImage);
		s.SetImage(IMU_NORMAL, imageManager->globalImages.flatNormal);
		s.shader = PRG_TEXTURED;

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

	void RenderSystem::RenderFrame(const emptyCommand_t* cmds)
	{
		backend->RenderCommandBuffer(cmds);
		++frameNum;
	}

	emptyCommand_t const* RenderSystem::SwapCommandBuffer_BeginNewFrame(bool smpMode)
	{
		emptyCommand_t* cmds = R_SwapCommandBuffers(smpMode);
		
		vertexCache->Frame();

		backend->unitRectSurface = unitRectSurface_;
		R_CreateSurfFormTris(unitRectSurface_, *unitRectTris);

		return cmds;
	}

	glm::vec2 RenderSystem::GetScreenSize() const
	{
		int w, h;
		backend->GetScreenSize(w, h);
		return glm::vec2(float(w), float(h));
	}

	uint8_t* R_FrameAlloc(uint32_t bytes)
	{
		bytes = CACHE_LINE_ALIGN(bytes);

		uint8_t* ret{};

		const int next = frameData->used.fetch_add(bytes, std::memory_order_relaxed);
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
			renderData = &frames[renderFrame];
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

		frameData->used.store(size, std::memory_order_relaxed);
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

	surface_t* R_CreateFullScreenRect()
	{
		const glm::vec3 coords[] = {
			{ 1.0f, 1.0f, 0.0f},
			{-1.0f, 1.0f, 0.0f},
			{-1.0f,-1.0f, 0.0f},
			{ 1.0f,-1.0f, 0.0f}
		};
		const glm::vec2 uv[] = {
			{ 1.0f, 1.0f},
			{ 0.0f, 1.0f},
			{ 0.0f, 0.0f},
			{ 1.0f, 0.0f}
		};
		const float white[] = { 1.0f,1.0f,1.0f,1.0f };

		auto* rect = new surface_t;

		rect->numVerts = 4;
		rect->numIndexes = 6;
		rect->verts = (drawVert_t*)MemAlloc16(sizeof(drawVert_t) * 4);
		rect->indexes = (elementIndex_t*)MemAlloc16(sizeof(elementIndex_t) * 6);
		rect->topology = TP_TRIANGLES;

		for (int i = 0; i < 4; ++i)
		{
			rect->verts[i].SetPos(coords[i]);
			rect->verts[i].SetUV(uv[i]);
			rect->verts[i].SetColor(white);
			rect->verts[i].SetNormal({ 0.0f,0.0f,1.0f });
			rect->verts[i].SetTangent({ 1.0f,0.0f,0.0f,1.0f });
		}

		const elementIndex_t idx[] = {1,2,0,0,2,3};
		for (int i = 0; i < 6; ++i)
		{
			rect->indexes[i] = idx[i];
		}

		rect->bounds.Extend({ -1.0f, -1.0f, 0.0f });
		rect->bounds.Extend({ 1.0f, 1.0f, 0.0f });

		return rect;
	}

	void R_CreateSurfFormTris(drawSurf_t& surf, surface_t& tris)
	{
		surf.frontEndGeo = &tris;
		if (tris.numIndexes == 0)
		{
			surf.numIndex = 0;
			return;
		}

		if ( ! renderSystem.vertexCache->IsCurrent(tris.vertexCache) )
		{
			tris.vertexCache = renderSystem.vertexCache->AllocTransientVertex(tris.verts, tris.numVerts * sizeof(tris.verts[0]));
		}
		if ( ! renderSystem.vertexCache->IsCurrent(tris.indexCache) )
		{
			tris.indexCache = renderSystem.vertexCache->AllocTransientIndex(tris.indexes, ( tris.numIndexes * sizeof(tris.indexes[0]) ) );
		}

		surf.indexCache = tris.indexCache;
		surf.vertexCache = tris.vertexCache;
		surf.numIndex = tris.numIndexes;

	}


	RenderSystem renderSystem;
}
