#include "RenderProgs.h"
#include "RenderSystem.h"
#include "Logger.h"

namespace jsr {

	ProgramManager::ProgramManager() :
		initialized(false),
		currentProgram(0),
		uniformsCache(0),
		uniforms()
	{
	}

	ProgramManager::~ProgramManager()
	{
		Shutdown();
	}

	bool ProgramManager::Init()
	{
		Info("size of uboUniforms_t = %d", sizeof(uboUniforms_t));
		if (LowLevelInit())
		{
			uniformsCache = renderSystem.vertexCache->AllocStaticUniform(&uniforms, sizeof(uniforms));

			initialized = true;
			return true;
		}

		return false;
	}
}
