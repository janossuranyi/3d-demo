#include "./Engine.h"
#include "./Logger.h"
#include "./RenderSystem.h"

namespace jsr {


	engineConfig_t::engineConfig_t()
	{
		r_resX = 1280;
		r_resY = 720;
		r_fullscreen = 0;
		r_debug = true;
		r_fbsrgb = false;
		r_fullscreen = 0;
		r_msaa = 0;
		r_vsync = 1;
	}

	void engineConfig_t::LoadFromFile()
	{
	}


	engineConfig_t engineConfig = engineConfig_t();

	Engine::Engine() : threaded(false)
	{
	}

	Engine::~Engine()
	{
		Shutdown();
	}

	bool Engine::Init(bool aThreaded)
	{
		if (!renderSystem.Init())
		{
			return false;
		}

		if (aThreaded)
		{
			if (!StartWorkerThread("JSR_Engine_thread"))
			{
				Error("Worker thread failed to start");
				return false;
			}
			threaded = true;
		}
		return true;
	}
	
	void Engine::Shutdown()
	{
		if (threaded)
		{
			StopThread(true);
		}
		
		renderSystem.Shutdown();
	}

	int Engine::Run()
	{
		return 0;
	}
}