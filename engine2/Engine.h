#pragma once
#include "./ThreadWorker.h"
#include "./Camera.h"
#include "./RenderWorld.h"

namespace jsr {

	struct engineConfig_t
	{
		int		r_resX;
		int		r_resY;
		int		r_fullscreen;
		int		r_vsync;
		int		r_msaa;
		bool	r_fbsrgb;
		bool	r_debug;
		bool	r_useStateCaching;
		engineConfig_t();
		void LoadFromFile();
	};

	class Engine : public ThreadWorker
	{
	public:
		Engine();
		virtual ~Engine();
		bool Init(bool aThreaded);
		bool LoadWorld(const std::string& filename);
		void MainLoop();
		void Shutdown();
		void GameLogic();
		int Run() override;

		Camera player{};

	private:
		RenderWorld* world;
		bool	threaded;
		int		lastNumDrawSurf;
	};

	extern engineConfig_t engineConfig;
}