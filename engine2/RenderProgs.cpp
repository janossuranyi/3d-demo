#include "RenderProgs.h"
#include "RenderSystem.h"

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
