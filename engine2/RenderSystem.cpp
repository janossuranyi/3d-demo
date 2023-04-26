#include "./RenderSystem.h"
#include "./Logger.h"

namespace jsr {
	RenderSystem::RenderSystem()
	{
		initialized		= false;
		frameNum		= 0;
		backend			= new RenderBackend();
		vertexCache		= new VertexCache();
		programManager	= new ProgramManager();
		imageManager	= new ImageManager();
	}
	RenderSystem::~RenderSystem()
	{
		if (IsInitialized())
		{
			Shutdown();
		}

		delete imageManager;
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

		if (!programManager->Init())	Error("[RenderSystem]: programManager init failed !");
		if (!vertexCache->Init())		Error("[RenderSystem]: vertexCache init failed !");
		if (!imageManager->Init())		Error("[RenderSystem]: imageManager init failed !");

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
