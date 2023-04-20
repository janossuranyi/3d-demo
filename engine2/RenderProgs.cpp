#include "RenderProgs.h"
#include "RenderSystem.h"

namespace jsr {

	renderProgram_t ProgramManager::builtins[] = {
			{"vertex_color",			SHADER_STAGE_DEFAULT,	LAYOUT_DRAW_VERT,	INVALID_PROGRAM },
			{"depth_pass",				SHADER_STAGE_DEFAULT,	LAYOUT_DRAW_VERT,	INVALID_PROGRAM },
			{"metallic_roughness_ao",	SHADER_STAGE_DEFAULT,	LAYOUT_DRAW_VERT,	INVALID_PROGRAM }
	};

	ProgramManager::ProgramManager()
	{
	}

	bool ProgramManager::Init()
	{
		if (renderSystem.IsInitialized() == false)
		{
			return false;
		}

		return LowLevelInit();
	}

}
