#include <SDL.h>
#include <chrono>
#include <thread>
#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl2.h>

#include "./Engine.h"
#include "./Logger.h"
#include "./RenderSystem.h"
#include "./Resources.h"
#include "./Math.h"
#include "./System.h"
#include "./Node3D.h"

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
		r_useStateCaching = true;
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
		Info("JSR Engine v0.9");
		Info("Platform: %s, PID: %d", jsr::GetPlatform(), std::this_thread::get_id());
		Info("Installed memory: %dMB", jsr::GetSystemRAM());

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

		if (!world->LoadMapFromGLTF(resourceManager->GetResource(filename)))
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
		player.MovementSpeed = 0.003f;
		player.MouseSensitivity = 0.1;
		player.Zoom = 75.0f;
		player.ProcessMouseMovement(0.f, 0.f);

		renderSystem.backend->SetClearColor(0.2f, 0.2f, 0.6f, 1.0f);
		player.Position = world->GetBounds().GetCenter();

		int x = 0, y = 0;

		bool mouseCapture = false;
		bool mover[4]{};
		bool angle[4]{};
		auto* cube = world->GetByName("Spot");
		//cube->SetScale({ .5f,.5f,.5f });

		ImGuiIO& io = ImGui::GetIO();
		world->exposure = 1.0f;
		bool spotOn = false;

		float angles[3]{};

		while (!quit)
		{
			glm::quat rotX = glm::angleAxis(glm::radians(angles[0]), normalize(vec3{1.0f,0.0f,0.0f}));
			glm::quat rotY = glm::angleAxis(glm::radians(angles[2]), normalize(vec3{0.0f,1.0f,0.0f}));
			glm::quat rotZ = glm::angleAxis(glm::radians(angles[1]), normalize(vec3{0.0f,0.0f,1.0f}));
			//cube->SetDir(rotX * rotY * rotZ);

			const emptyCommand_t* cmds = renderSystem.SwapCommandBuffer_BeginNewFrame(this->threaded);

			SignalWork();

			ImGui::NewFrame();
			ImGui::LabelText("Visible surfaces", "%d", lastNumDrawSurf);
			ImGui::DragFloat("Exposure", &world->exposure, 0.05f, 0.1f, 20.0f);
			ImGui::DragFloat("Shadow Scale", &renderGlobals.shadowScale, 0.01f, 0.1f, 1.0f);
			ImGui::DragFloat("Shadow Bias", &renderGlobals.shadowBias, 0.0001f, 0.0001f, 0.005f, "%.5f");

			ImGui::ColorEdit3("Ambient light", &renderGlobals.ambientColor.r);
			ImGui::DragFloat("Ambient scale", &renderGlobals.ambientScale, 0.001f, 0.001f, 1.0f);

			renderSystem.RenderFrame(cmds);

			while (SDL_PollEvent(&e) != SDL_FALSE)
			{
				if (io.WantCaptureMouse || io.WantCaptureKeyboard)
				{
					ImGui_ImplSDL2_ProcessEvent(&e);
					continue;
				}

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
					case SDLK_LEFT:
						angle[0] = false;
						break;
					case SDLK_RIGHT:
						angle[1] = false;
						break;
					case SDLK_UP:
						angle[2] = false;
						break;
					case SDLK_DOWN:
						angle[3] = false;
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
					case SDLK_ESCAPE:
						quit = true;
						break;
					case SDLK_LEFT:
						angle[0] = true;
						break;
					case SDLK_RIGHT:
						angle[1] = true;
						break;
					case SDLK_UP:
						angle[2] = true;
						break;
					case SDLK_DOWN:
						angle[3] = true;
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

			if (angle[0])
			{
				angles[0] += 1.f;
				angles[0] = std::fmodf(angles[0], 360.f);
			}
			if (angle[1])
			{
				angles[0] -= 1.f;
				angles[0] = std::fmodf(angles[0], 360.f);
			}
			if (angle[2])
			{
				angles[1] += 1.f;
				angles[1] = std::fmodf(angles[1], 360.f);
			}
			if (angle[3])
			{
				angles[1] -= 1.f;
				angles[1] = std::fmodf(angles[1], 360.f);
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

	void Engine::GameLogic()
	{
		using namespace glm;

		int x, y;
		renderSystem.backend->GetScreenSize(x, y);

		alignas(16) uboFreqLowVert_t vertUbo{};
		alignas(16) uboFreqLowFrag_t fragUbo{};

		//const float R = world->GetBounds().GetRadius() * 4.0f;
		constexpr float R = 500.0f;
		mat4 projMatrix = glm::perspective(glm::radians(player.Zoom), float(x) / float(y), 0.1f, R);
		mat4 viewMatrix = player.GetViewMatrix();
		mat4 vpMatrix = projMatrix * viewMatrix;

		viewDef_t* view = (viewDef_t*)R_FrameAlloc(sizeof(*view));

		view->exposure = world->exposure;
		view->farClipDistance = R;
		view->nearClipDistance = 0.1f;

		view->renderView.viewID = 1;
		view->renderView.fov = radians(player.Zoom * 1.5f);
		view->renderView.vieworg = player.Position;
		view->renderView.viewMatrix = viewMatrix;
		view->projectionMatrix = projMatrix;
		view->isSubview = false;
		view->isMirror = false;
		view->frustum = Frustum(projMatrix);
		view->unprojectionToCameraMatrix = glm::inverse(projMatrix);
		view->unprojectionToWorldMatrix = glm::inverse(vpMatrix);
		view->viewport = { 0,0,x,y };
		view->scissor = view->viewport;
		view->renderWorld = world;

		vertUbo.viewMatrix = view->renderView.viewMatrix;
		vertUbo.projectMatrix = view->projectionMatrix;
		fragUbo.nearFarClip = { view->nearClipDistance,view->farClipDistance,0.f,0.f };
		fragUbo.screenSize = { float(x), float(y), 1.0f / (float)x, 1.0f / (float)y };
		fragUbo.params.x = view->exposure;
		fragUbo.viewOrigin = vec4(0.f, 0.f, 0.f, 1.f);
		fragUbo.invProjMatrix = view->unprojectionToCameraMatrix;
		fragUbo.ambientColor = { renderGlobals.ambientColor, renderGlobals.ambientScale };

		view->freqLowVert = renderSystem.vertexCache->AllocTransientUniform(&vertUbo, sizeof(vertUbo));
		view->freqLowFrag = renderSystem.vertexCache->AllocTransientUniform(&fragUbo, sizeof(fragUbo));		

		world->RenderView(view);
		lastNumDrawSurf = view->numDrawSurfs;

	}

	int Engine::Run()
	{
		GameLogic();
		return 0;
	}
}