#include "./RenderBackend.h"
#include "./RenderProgs.h"
#include "./Logger.h"

#include <memory>

namespace jsr {

	glcontext_t glcontext{};

	backendCounters_t perfCounters;

	RenderBackend::RenderBackend()
	{
		glcontext.frameCounter = 0;
		std::memset(glcontext.tmu, 0xffff, sizeof(glcontext.tmu));
		currenttmu = 0;
		currentFramebuffer = nullptr;
		clearColor[0] = .0f;
		clearColor[1] = .0f;
		clearColor[2] = .0f;
		clearColor[3] = 1.0f;
		for (int i = 0; i < MAX_BINDING; ++i) glcontext.vtxBindings[i] = { 0,0,0 };
		initialized = false;
		view = {};
	}

	bool RenderBackend::IsInitialized() const
	{
		return initialized;
	}

	tmu_t* RenderBackend::GetTextureUnit(int index)
	{
		return &glcontext.tmu[index];
	}

	RenderBackend::~RenderBackend()
	{
		Shutdown();
	}

	bool RenderBackend::Init()
	{
		Info("---- Init graphics subsystem ----");
		if (!R_InitGfxAPI())
		{
			return false;
		}
		initialized = true;
		return true;
	}

	void RenderBackend::Shutdown()
	{
		if (initialized)
		{
			R_ShutdownGfxAPI();
			initialized = false;
		}
	}

	glm::vec4 RenderBackend::GetClearColor() const
	{
		return glm::vec4(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
	}

	int RenderBackend::GetCurrentTextureUnit() const
	{
		return currenttmu;
	}
}
