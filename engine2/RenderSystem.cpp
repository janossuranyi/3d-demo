#include "./RenderSystem.h"

namespace jsr {
	RenderSystem::RenderSystem()
	{
		initialized		= false;
		frameNum		= 0;
		backend			= new RenderBackend();
		vertexCache		= new VertexCache();
		programManager	= new ProgramManager();
	}
	RenderSystem::~RenderSystem()
	{
		if (IsInitialized())
		{
			Shutdown();
		}

		delete programManager;
		delete vertexCache;
		delete backend;

		initialized = false;
	}

	bool RenderSystem::Init()
	{
		if (!backend->Init())
		{
			return false;
		}

		programManager->Init();
		vertexCache->Init();

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

	void RenderSystem::Frame()
	{

		programManager->UpdateUniforms();
		vertexCache->Frame();

		backend->Clear(true, true, true);

		programManager->BindUniforms();

		/*
		RENDERING
		*/
		backend->EndFrame();
		++frameNum;
	}


	RenderSystem renderSystem;

}
