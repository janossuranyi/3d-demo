#include "RenderProgs.h"
#include "RenderSystem.h"
#include "Logger.h"

namespace jsr {

	ProgramManager::ProgramManager() :
		initialized(false),
		currentProgram(0),
		uniformsCache(0),
		uniforms(),
		g_freqLowVert(),
		g_freqHighVert(),
		g_freqLowFrag(),
		g_freqHighFrag(),
		c_freqLowVert(),
		c_freqHighVert(),
		c_freqLowFrag(),
		c_freqHighFrag(),
		uboChangedBits()
	{
	}

	ProgramManager::~ProgramManager()
	{
		Shutdown();
	}

	void ProgramManager::UniformChanged(eUboBufferBinding b)
	{
		uboChangedBits |= (1UL << b);
	}

	bool ProgramManager::Init()
	{
		Info("size of uboUniforms_t = %d", sizeof(uboUniforms_t));
		if (LowLevelInit())
		{
			memset(&uniforms, 0, sizeof(uniforms));
			uniformsCache = renderSystem.vertexCache->AllocStaticUniform(&uniforms, sizeof(uniforms));

			c_freqLowVert = renderSystem.vertexCache->AllocStaticUniform(&g_freqLowVert, sizeof(g_freqLowVert));
			c_freqHighVert = renderSystem.vertexCache->AllocStaticUniform(&g_freqHighVert, sizeof(g_freqHighVert));
			c_freqLowFrag = renderSystem.vertexCache->AllocStaticUniform(&g_freqLowFrag, sizeof(g_freqLowFrag));
			c_freqHighFrag = renderSystem.vertexCache->AllocStaticUniform(&g_freqHighFrag, sizeof(g_freqHighFrag));

			BindUniforms();

			initialized = true;
			return true;
		}

		return false;
	}
}
