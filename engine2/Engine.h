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
		int Run() override;

		Camera player{};

	private:
		RenderWorld* world;
		bool	threaded;
	};

	extern engineConfig_t engineConfig;
}