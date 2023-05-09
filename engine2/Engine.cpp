#include <SDL.h>
#include <chrono>
#include <thread>
#include "./Engine.h"
#include "./Logger.h"
#include "./RenderSystem.h"
#include "./Math.h"

namespace jsr {

	using namespace std::chrono;

	using namespace std::chrono_literals;

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

		player.Position = glm::vec3(0.0f, 5.0f, -10.f);
		
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
		std::atomic_bool quit{};

		SDL_Event e;
		float time, dt;
		dt = 0.0f;
		time = (float)SDL_GetTicks();
		player.MovementSpeed = 0.006f;
		player.MouseSensitivity = 0.01;

		int x=0, y=0, px=0, py=0;

		bool mouseCapture = false;

		while (!quit)
		{
			renderSystem.Frame();

			while (SDL_PollEvent(&e) != SDL_FALSE)
			{
				if (e.type == SDL_QUIT)
				{
					quit = true;
					break;
				}
				if (e.type == SDL_KEYDOWN)
				{
					auto key = e.key.keysym.sym;
					switch (key)
					{
					case SDLK_w:
						player.ProcessKeyboard(FORWARD, dt);
						break;
					case SDLK_s:
						player.ProcessKeyboard(BACKWARD, dt);
						break;
					case SDLK_a:
						player.ProcessKeyboard(LEFT, dt);
						break;
					case SDLK_d:
						player.ProcessKeyboard(RIGHT, dt);
						break;
					case SDLK_ESCAPE:
						quit = true;
						break;
					}
					Info("x: %f, y: %f, z: %f", player.Position.x, player.Position.y, player.Position.z);
				}
				else if (e.type == SDL_MOUSEBUTTONDOWN && !mouseCapture)
				{
					SDL_SetRelativeMouseMode(SDL_TRUE);
					mouseCapture = true;
				}
				else if (e.type == SDL_MOUSEBUTTONUP)
				{
					SDL_SetRelativeMouseMode(SDL_FALSE);
					mouseCapture = false;
				}
			}

			if (mouseCapture)
			{
				SDL_GetRelativeMouseState(&x, &y);
				player.ProcessMouseMovement((float)x, (float)y, true);
				//Info("mx: %d, my: %d", x, y);
			}

			float now = (float)SDL_GetTicks();
			dt = now - time;
			time = now;

			//std::this_thread::yield();
		}
		SDL_SetRelativeMouseMode(SDL_FALSE);

		return 0;
	}
}