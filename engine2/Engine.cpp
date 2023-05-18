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

		player.Position = glm::vec3(0.f, 0.f, 5.f);
		player.Front = glm::vec3(0.f, 0.f, -1.f);
		
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
		using namespace glm;
		std::atomic_bool quit{};

		SDL_Event e;
		float time, dt;
		dt = 0.0f;
		time = (float)SDL_GetTicks();
		player.MovementSpeed = 0.001f;
		player.MouseSensitivity = 0.1;
		player.Zoom = 80.0f;
		player.ProcessMouseMovement(0.f, 0.f);

		renderSystem.backend->SetClearColor(0.2f, 0.2f, 0.6f, 1.0f);
		player.Position = world->GetBounds().GetCenter();

		int x = 0, y = 0;

		bool mouseCapture = false;
		bool mover[4]{};

		while (!quit)
		{


			emptyCommand_t* cmds = R_SwapCommandBuffers(this->threaded);
			SignalWork();

			renderSystem.Frame(cmds);

			while (SDL_PollEvent(&e) != SDL_FALSE)
			{
				if (e.type == SDL_QUIT)
				{
					quit = true;
					break;
				}
				if (e.type == SDL_KEYUP)
				{
					auto key = e.key.keysym.sym;
					switch (key)
					{
					case SDLK_w:
						mover[FORWARD] = false;
						break;
					case SDLK_s:
						mover[BACKWARD] = false;
						break;
					case SDLK_a:
						mover[LEFT] = false;
						break;
					case SDLK_d:
						mover[RIGHT] = false;
						break;
					}
				}

				if (e.type == SDL_KEYDOWN)
				{
					auto key = e.key.keysym.sym;
					switch (key)
					{
					case SDLK_w:
						mover[FORWARD] = true;
						break;
					case SDLK_s:
						mover[BACKWARD] = true;
						break;
					case SDLK_a:
						mover[LEFT] = true;
						break;
					case SDLK_d:
						mover[RIGHT] = true;
						break;
					case SDLK_0:
						renderSystem.programManager->uniforms.debugFlags = vec4(0.f);
						break;
					case SDLK_1:
						renderSystem.programManager->uniforms.debugFlags = vec4(1.f);
						break;
					case SDLK_2:
						renderSystem.programManager->uniforms.debugFlags = vec4(2.f);
						break;
					case SDLK_3:
						renderSystem.programManager->uniforms.debugFlags = vec4(4.f);
						break;
					case SDLK_4:
						renderSystem.programManager->uniforms.debugFlags = vec4(8.f);
						break;
					case SDLK_5:
						renderSystem.programManager->uniforms.debugFlags = vec4(16.f);
						break;
					case SDLK_6:
						renderSystem.programManager->uniforms.debugFlags = vec4(32.f);
						break;
					case SDLK_ESCAPE:
						quit = true;
						break;
					}
					//Info("x: %f, y: %f, z: %f", player.Position.x, player.Position.y, player.Position.z);
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
				else if (e.type == SDL_MOUSEWHEEL)
				{
					float r = (float)e.wheel.y;
					if (e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) r = -r;
					player.ProcessMouseScroll((float)e.wheel.y);
				}
			}

			SDL_GetRelativeMouseState(&x, &y);
			if (mouseCapture)
			{
				player.ProcessMouseMovement((float)x, (float)-y, true);
			}

			if (mover[FORWARD])		player.ProcessKeyboard(FORWARD, dt);
			if (mover[BACKWARD])	player.ProcessKeyboard(BACKWARD, dt);
			if (mover[LEFT])		player.ProcessKeyboard(LEFT, dt);
			if (mover[RIGHT])		player.ProcessKeyboard(RIGHT, dt);

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

	struct
	{
		bool operator()(drawSurf_t* a, drawSurf_t* b) const { return (uint32)a->sort < (uint32)b->sort; }
	} drawSurfLess;

	int Engine::Run()
	{
		using namespace glm;
		
		int x, y;
		renderSystem.backend->GetScreenSize(x, y);

		mat4 projMatrix = glm::perspective(glm::radians(player.Zoom), float(x) / float(y), 0.1f, 1000.0f);
		mat4 viewMatrix = player.GetViewMatrix();
		mat4 vpMatrix = projMatrix * viewMatrix;

		renderSystem.programManager->uniforms.clipPlanes = vec4(0.1f, 1000.0f, 0.0f, 0.0f);
		renderSystem.programManager->uniforms.viewOrigin = vec4(player.Position, 1.0f);

		viewDef_t* view = (viewDef_t *)R_FrameAlloc(sizeof(*view));
		view->renderView.viewID = 1;
		view->renderView.fov = player.Zoom;
		view->renderView.vieworg = player.Position;
		view->renderView.viewMatrix = viewMatrix;
		view->projectionMatrix = projMatrix;
		view->isSubview = false;
		view->isMirror = false;
		view->frustum = Frustum(projMatrix);
		view->unprojectionToCameraMatrix = glm::inverse(projMatrix);
		view->unprojectionToWorldMatrix = glm::inverse(vpMatrix);
		view->viewport = screenRect_t{ 0,0,x,y };
		view->scissor = view->viewport;
		view->renderWorld = world;

		world->RenderView(view);

		if (view->numDrawSurfs)
		{
			// allocate drawSurf pointers
			view->drawSurfs = (drawSurf_t**)R_FrameAlloc(view->numDrawSurfs * sizeof(view->drawSurfs));
			int i = 0;
			for (const auto* ent = view->viewEntites; ent != nullptr; ent = ent->next)
			{
				for (auto* drawSurf = ent->surf; drawSurf != nullptr; drawSurf = drawSurf->next)
				{
					view->drawSurfs[i++] = drawSurf;
				}
			}

			std::sort(view->drawSurfs, view->drawSurfs + view->numDrawSurfs, drawSurfLess);
			drawViewCommand_t* cmd = (drawViewCommand_t*)R_GetCommandBuffer(sizeof(*cmd));
			cmd->command = RC_DRAW_VIEW;
			cmd->view = view;

			//Info("Surfaces to draw: %d", view->numDrawSurfs);
		}
		else
		{
			//Info("No surface to draw !");
		}

		return 0;
	}
}