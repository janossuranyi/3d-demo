#include "RenderProgs.h"
#include "RenderSystem.h"
#include "Logger.h"

namespace jsr {

	ProgramManager::ProgramManager() :
		initialized(false),
		currentProgram(0)
	{
	}

	ProgramManager::~ProgramManager()
	{
		Shutdown();
	}

	void ProgramManager::BindUniformBlock(eUboBufferBinding binding, vertCacheHandle_t handle)
	{
		UniformBuffer buffer;
		if (renderSystem.vertexCache->GetUniformBuffer(handle, buffer))
		{
			BindUniformBlock(binding, buffer);
		}
	}

	bool ProgramManager::Init()
	{
		if (LowLevelInit())
		{
			initialized = true;
			return true;
		}

		return false;
	}
}
