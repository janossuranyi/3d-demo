#include "./RenderSystem.h"

namespace jsr {
	RenderSystem::RenderSystem()
	{
		initialized = false;
		frameNum = 0;
		backend = new RenderBackend();
		vertexCache = new VertexCache();
	}
	RenderSystem::~RenderSystem()
	{
		if (IsInitialized())
		{
			Shutdown();
		}
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

		backend->Clear(true, true, true);
		vertexCache->Frame();
		/*
		RENDERING
		*/
		backend->EndFrame();
		++frameNum;
	}


	RenderSystem renderSystem;

}
