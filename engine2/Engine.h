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
		bool	r_bloom;
		bool	r_fxaa;
		bool	r_ssao;
		float	r_ssao_radius;
		float	r_ssao_bias;
		float	r_ssao_str;
		float	r_gamma;
		int		r_pp;
		int		r_pp_offset;
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
		int		lastNumDrawShadowSurf;

		float bloomParams2_y{ 0.0f };
	};

	extern engineConfig_t engineConfig;
}