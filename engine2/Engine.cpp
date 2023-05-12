#include <SDL.h>
#include <chrono>
#include <thread>
#include "./Engine.h"
#include "./Logger.h"
#include "./RenderSystem.h"
#include "./Resources.h"
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

	Engine::Engine() : threaded(false), world()
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

		player.Position = glm::vec3(0.f, 0.f, 0.f);
		
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

	bool Engine::LoadWorld(const std::string& filename)
	{
		if (world) delete world;
		
		world = new RenderWorld();

		if (!world->LoadMapFromGLTF(resourceMgr->GetResource(filename)))
		{
			Error("Error loading map!");
			return false;
		}

		return true;
	}

	void Engine::MainLoop()
	{
		std::atomic_bool quit{};

		SDL_Event e;
		float time, dt;
		dt = 0.0f;
		time = (float)SDL_GetTicks();
		player.MovementSpeed = 0.006f;
		player.MouseSensitivity = 0.01;

		int x = 0, y = 0, px = 0, py = 0;

		bool mouseCapture = false;

		while (!quit)
		{
			emptyCommand_t* cmds = R_SwapCommandBuffers(this->threaded);

			SignalWork();

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
			WaitForThread();
		}
		SDL_SetRelativeMouseMode(SDL_FALSE);
	}
	
	void Engine::Shutdown()
	{
		if (threaded)
		{
			StopThread(true);
		}
		
		if (world) delete world;
		world = nullptr;

		renderSystem.Shutdown();
	}

	int Engine::Run()
	{
		int x, y;
		renderSystem.backend->GetScreenSize(x, y);

		glm::mat4 projMatrix = glm::perspective(glm::radians(player.Zoom), float(x) / float(y), 0.1f, 1000.0f);
		glm::mat4 viewMatrix = player.GetViewMatrix();
		glm::mat4 vpMatrix = projMatrix * viewMatrix;

		viewDef_t* view = (viewDef_t *)R_FrameAlloc(sizeof(viewDef_t));
		view->renderView.viewID = 1;
		view->renderView.fov = player.Zoom;
		view->renderView.vieworg = player.Position;
		view->renderView.viewaxis = glm::mat3(viewMatrix);
		view->projectionMatrix = projMatrix;
		view->isSubview = false;
		view->isMirror = false;
		view->frustum = Frustum(projMatrix, glm::mat4(1.0f));
		view->unprojectionToCameraMatrix = glm::inverse(projMatrix);
		view->unprojectionToWorldMatrix = glm::inverse(vpMatrix);
		view->viewport = screenRect_t{ 0,0,x,y };
		view->scissor = view->viewport;
		view->renderWorld = world;

		world->RenderView(view);

		return 0;
	}
}