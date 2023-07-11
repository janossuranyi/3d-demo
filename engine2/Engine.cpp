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
#include "./TaskExecutor.h"

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
		r_ssao_radius = 0.25f;
		r_ssao_bias = 0.001;
		r_ssao_str = 1.0f;
		r_ssao = false;
		r_gamma = 2.2f;
		r_pp = 2;
		r_pp_offset = 1;
	}

	void engineConfig_t::LoadFromFile()
	{
	}


	engineConfig_t engineConfig = engineConfig_t();

	Engine::Engine() : threaded(false), world()
	{
		lastNumDrawShadowSurf = 0;
		lastNumDrawSurf = 0;
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

		taskManager.Init();
		
		player.Position = glm::vec3(0.f, 0.f, 5.f);
		player.Front = glm::vec3(0.f, -1.f, 0.f);

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
		player.MovementSpeed = 0.001f;
		player.MouseSensitivity = 0.1;
		player.Zoom = 55.0f;
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
		world->exposure = 1.3f;
		bool spotOn = false;

		float angles[3]{};
		uint frameTimeCounter = 100;
		float frameTime{};
		
		while (!quit)
		{
			glm::quat rotX = glm::angleAxis(glm::radians(angles[0]), normalize(vec3{1.0f,0.0f,0.0f}));
			glm::quat rotY = glm::angleAxis(glm::radians(angles[2]), normalize(vec3{0.0f,1.0f,0.0f}));
			glm::quat rotZ = glm::angleAxis(glm::radians(angles[1]), normalize(vec3{0.0f,0.0f,1.0f}));
			//cube->SetDir(rotX * rotY * rotZ);

			const emptyCommand_t* cmds = renderSystem.SwapCommandBuffer_BeginNewFrame(this->threaded);

			if ((frameTimeCounter--) == 0)
			{
				frameTime = dt;
				frameTimeCounter = 100;
			}

			SignalWork();

			ImGui::NewFrame();
			ImGui::LabelText("Visible surfaces", "%d", lastNumDrawSurf);
			ImGui::LabelText("Visible shadow surfaces", "%d", lastNumDrawShadowSurf);
			ImGui::LabelText("Frame time", "%.2f", frameTime);
			ImGui::DragFloat("Bloom scale", &renderGlobals.bloomScale, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Bloom treshold", &bloomParams2_y, 0.01f, 0.0f, 20.0f);

			ImGui::DragFloat("SSAO radius", &engineConfig.r_ssao_radius, 0.01f, 0.0f, 10.0f);
			ImGui::DragFloat("SSAO bias", &engineConfig.r_ssao_bias, 0.001f, 0.0f, 10.0f);
			ImGui::DragFloat("SSAO strength", &engineConfig.r_ssao_str, 0.01f, 0.01f, 10.0f);

			ImGui::DragFloat("Exposure", &world->exposure, 0.05f, 0.1f, 20.0f);
			ImGui::DragFloat("Shadow Scale", &renderGlobals.shadowScale, 0.01f, 0.1f, 1.0f);
			ImGui::DragFloat("Shadow Bias", &renderGlobals.shadowBias, 0.00001f, 0.00001f, 0.005f, "%.5f");

			ImGui::ColorEdit3("Ambient light", &renderGlobals.ambientColor.r);
			ImGui::DragFloat("Ambient scale", &renderGlobals.ambientScale, 0.001f, 0.001f, 1.0f);
			ImGui::DragFloat("Gamma", &engineConfig.r_gamma, 0.01f, 1.0f, 3.0f);
			ImGui::Checkbox("Bloom", &engineConfig.r_bloom);
			ImGui::Checkbox("FXAA", &engineConfig.r_fxaa);
			ImGui::Checkbox("SSAO", &engineConfig.r_ssao);

			ImGui::DragInt("CONV Type", &engineConfig.r_pp, 0.2f, 0, 8);
			ImGui::DragInt("CONV Offset", &engineConfig.r_pp_offset, 0.2f, 1, 7);

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
		taskManager.Shutdown();
	}

	void Engine::GameLogic()
	{
		using namespace glm;

		int x, y;
		renderSystem.backend->GetScreenSize(x, y);
		float fx = float(x), fy = float(y);

		alignas(16) uboVSViewParams_t vertUbo{};
		alignas(16) uboFSViewParams_t fragUbo{};
		alignas(16) uboLightData_t sunLight {};

		//const float R = world->GetBounds().GetRadius() * 4.0f;
		constexpr float R = 500.0f;
		const mat4 projMatrix = perspective(radians(player.Zoom), fx / fy, 0.1f, R);
//		const auto b = world->GetBounds();
//		const mat4 projMatrix = ortho(0.0f, b.GetMax().x, 0.0f, b.GetMax().y, 0.1f, R);
		const mat4 viewMatrix = player.GetViewMatrix();
		const mat4 vpMatrix = projMatrix * viewMatrix;
		viewDef_t* view = (viewDef_t*)R_FrameAlloc(sizeof(*view));

		view->viewSunLight = (viewLight_t*)R_FrameAlloc(sizeof(*view->viewSunLight));
		view->exposure = world->exposure;
		view->farClipDistance = R;
		view->nearClipDistance = 0.1f;

		view->renderView.viewID = 1;
		view->renderView.fov = radians(player.Zoom);
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

		view->viewSunLight->origin = viewMatrix * normalize(vec4(0.0f, 1.0f, 2.0f, 0.0f));
		view->viewSunLight->shader = PRG_DEFERRED_DIR_LIGHT;
		view->viewSunLight->color = vec4(1.0f, 0.3f, 0.0f, 0.01f);
		view->viewSunLight->type = LIGHT_DIRECTED;
		//mat4 sunOrtho = glm::ortho()
		sunLight.color = view->viewSunLight->color;
		sunLight.origin = vec4(view->viewSunLight->origin,0.0f);
		sunLight.projectMatrix = mat4(1.0f);
		sunLight.shadowparams = vec4(0.f);
		view->viewSunLight->lightData = renderSystem.vertexCache->AllocTransientUniform(&sunLight, sizeof(sunLight));

		int bloomDiv = 1 << renderGlobals.bloomDownsampleLevel;
		vertUbo.viewMatrix = view->renderView.viewMatrix;
		vertUbo.projectMatrix = view->projectionMatrix;
		vertUbo.invProjectMatrix = view->unprojectionToCameraMatrix;
		fragUbo.nearFarClip = { view->nearClipDistance,view->farClipDistance,0.f,0.f };
		fragUbo.screenSize = { float(x), float(y), 1.0f / (float)x, 1.0f / (float)y };
		fragUbo.params.x = view->exposure;
		fragUbo.params.y = engineConfig.r_gamma;
		fragUbo.viewOrigin = vec4(0.f, 0.f, 0.f, 1.f);
		fragUbo.invProjMatrix = (view->unprojectionToCameraMatrix);
		fragUbo.projectMatrix = projMatrix;
		fragUbo.ambientColor = { renderGlobals.ambientColor, renderGlobals.ambientScale };
		fragUbo.bloomParams.x = 2.0f;
		fragUbo.bloomParams.y = engineConfig.r_bloom ? renderGlobals.bloomScale : 0.0f;
		fragUbo.bloomParams.z = 1.0f / (float(x) / bloomDiv);
		fragUbo.bloomParams.w = 1.0f / (float(y) / bloomDiv);
		fragUbo.bloomParams2.x = 0.4f;
		fragUbo.bloomParams2.y = bloomParams2_y;

		view->VS_ViewParams = renderSystem.vertexCache->AllocTransientUniform(&vertUbo, sizeof(vertUbo));
		view->FS_ViewParams = renderSystem.vertexCache->AllocTransientUniform(&fragUbo, sizeof(fragUbo));		

		world->RenderView(view);
		lastNumDrawSurf = view->numDrawSurfsNotLight;
		lastNumDrawShadowSurf = view->numDrawSurfsShadow;

	}

	int Engine::Run()
	{
		GameLogic();
		return 0;
	}
}