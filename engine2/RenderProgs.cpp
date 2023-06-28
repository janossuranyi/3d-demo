#include "RenderProgs.h"
#include "RenderSystem.h"
#include "Logger.h"

namespace jsr {

	ProgramManager::ProgramManager() :
		initialized(false),
		g_commonData(),
		g_commonData_h(),
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

	void ProgramManager::BindCommonUniform()
	{
		BindUniformBlock(UBB_COMMON_DATA, g_commonData_h);
	}

	bool ProgramManager::Init()
	{
		if (LowLevelInit())
		{
			initialized = true;

			memset(&g_commonData, 0, sizeof(g_commonData));
			g_commonData_h = renderSystem.vertexCache->AllocStaticUniform(&g_commonData, sizeof(g_commonData));
			BindCommonUniform();

			memset(&g_sharedData, 0, sizeof(g_sharedData));
			g_backendData_h = renderSystem.vertexCache->AllocStaticUniform(&g_sharedData, sizeof(g_sharedData));
			BindUniformBlock(UBB_SHARED_DATA, g_backendData_h);

			return true;
		}


		return false;
	}
}
