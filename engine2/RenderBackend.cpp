#include "RenderBackend.h"
#include "Logger.h"

#include <memory>

namespace jsr {

	glcontext_t glcontext{};

	RenderBackend::RenderBackend()
	{
		glcontext.frameCounter = 0;
		std::memset(glcontext.tmu, 0xffff, sizeof(glcontext.tmu));
		currenttmu = 0;
		currentLayout = VL_NONE;
		currentFramebuffer = nullptr;
		clearColor[0] = .0f;
		clearColor[1] = .0f;
		clearColor[2] = .0f;
		clearColor[3] = 1.0f;
		initialized = false;
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
		SetClearColor(0, 0, 0, 1);
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

	void RenderBackend::SetClearColor(float r, float g, float b, float a)
	{
		clearColor[0] = r;
		clearColor[1] = g;
		clearColor[2] = b;	
		clearColor[3] = a;
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
