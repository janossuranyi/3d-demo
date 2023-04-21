#pragma once
#include "./ThreadWorker.h"

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
		void Shutdown();
		int Run() override;
	private:
		bool	threaded;
	};

	extern engineConfig_t engineConfig;
}