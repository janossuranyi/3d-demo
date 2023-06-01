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

		world->LoadModelsFromGLTF(resourceManager->GetResource("models/zeroOneCube.glb"));

		if (!world->LoadMapFromGLTF(resourceManager->GetResource(filename)))
		{
			Error("Error loading map!");
			return false;
		}

		RenderModel* test = renderSystem.modelManager->CreateModel("_testmodel");
		test->CreateFromTris(renderSystem.unitSphereTris);
		test->UpdateSurfaceCache();
		world->InsertNode("testnode", test, { 0.0f,1.2f,0.0f });		
//		Material* m = renderSystem.materialManager->FindMaterial("Material_15");

		test->GetSurface(0)->shader = renderSystem.defaultMaterial;

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
		player.Zoom = 75.0f;
		player.ProcessMouseMovement(0.f, 0.f);

		renderSystem.backend->SetClearColor(0.2f, 0.2f, 0.6f, 1.0f);
		player.Position = world->GetBounds().GetCenter();

		int x = 0, y = 0;

		bool mouseCapture = false;
		bool mover[4]{};
		float angle1 = 0.0f;
		auto* cube = world->GetByName("testnode");
		cube->SetScale({ .5f,.5f,.5f });

		ImGuiIO& io = ImGui::GetIO();
		world->exposure = 1.0f;
		world->lightColor = vec4(vec3(255.f, 87.f, 51.f) / 255.0f, 8.0f);
		world->lightAttenuation = vec4(1.0f, 0.0f, 1.0f, 0.0f);
		world->spotLightParams.x = 40.0f;
		world->spotLightParams.y = 35.0f;
		world->spotLightParams.z = 0.5f;
		world->spotLightParams.w = 0.0f;
		bool spotOn = false;

		while (!quit)
		{
			glm::quat rotY = glm::angleAxis(glm::radians(angle1), normalize(vec3{ 0.0f,1.0f, 0.0f }));
			glm::quat rotX = glm::angleAxis(glm::radians(-90.f), normalize(vec3{ 1.0f,0.0f, 0.0f }));
			cube->SetDir(rotY * rotX);

			const emptyCommand_t* cmds = renderSystem.SwapCommandBuffer_BeginNewFrame(this->threaded);

			SignalWork();

			ImGui::NewFrame();
			ImGui::LabelText("Visible surfaces", "%d", lastNumDrawSurf);
			ImGui::DragFloat("Exposure", &world->exposure, 0.05f, 0.1f, 20.0f);
			ImGui::DragFloat("Attn. Kl", &world->lightAttenuation.y, 0.05f, 0.0f, 100.0f);
			ImGui::DragFloat("Attn. Kq", &world->lightAttenuation.z, 0.01f, 0.0f, 100.0f);
			ImGui::DragFloat("Spot Exp", &world->spotLightParams.z, 0.01f, 0.0f, 100.0f);
			ImGui::DragFloat("Spot Cone", &world->spotLightParams.x, 0.1f, 0.0f, 120.0f);
			ImGui::DragFloat("Spot Inner", &world->spotLightParams.y, 0.1f, 0.0f, 120.0f);
			ImGui::DragFloat("Shadow Scale", &renderGlobals.shadowScale, 0.01f, 0.1f, 1.0f);
			ImGui::DragFloat("Shadow Bias", &renderGlobals.shadowBias, 0.0001f, 0.0001f, 0.005f, "%.5f");
			ImGui::Checkbox("Spot On", &spotOn);
			ImGui::ColorEdit3("Light color", &world->lightColor.x);
			ImGui::DragFloat("Light power", &world->lightColor.w, 0.02f, 0.2f, 1000.0f);

			renderSystem.RenderFrame(cmds);

			world->spotLightParams.w = spotOn ? 1.0f : 0.0f;

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
						renderSystem.programManager->g_freqLowFrag.params.y = 0.0f;
						break;
					case SDLK_1:
						renderSystem.programManager->g_freqLowFrag.params.y = 1.0f;
						break;
					case SDLK_2:
						renderSystem.programManager->g_freqLowFrag.params.y = 2.0f;
						break;
					case SDLK_3:
						renderSystem.programManager->g_freqLowFrag.params.y = 3.0f;
						break;
					case SDLK_4:
						renderSystem.programManager->g_freqLowFrag.params.y = 4.0f;
						break;
					case SDLK_5:
						renderSystem.programManager->g_freqLowFrag.params.y = 5.0f;
						break;
					case SDLK_6:
						renderSystem.programManager->g_freqLowFrag.params.y = 6.0f;
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

			angle1 += .02f * dt;
			angle1 = std::fmodf(angle1, 360.f);

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
		using namespace glm;
		
		int x, y;
		renderSystem.backend->GetScreenSize(x, y);
		
		//const float R = world->GetBounds().GetRadius() * 4.0f;
		constexpr float R = 1000.0f;
		mat4 projMatrix = glm::perspective(glm::radians(player.Zoom), float(x) / float(y), 0.1f, R);
		mat4 viewMatrix = player.GetViewMatrix();
		mat4 vpMatrix = projMatrix * viewMatrix;

		viewDef_t* view = (viewDef_t *)R_FrameAlloc(sizeof(*view));

		view->exposure = world->exposure;
		view->farClipDistance = R;
		view->nearClipDistance = 0.1f;
		view->lightColor = world->lightColor;
		view->lightPos = vec4(player.Position + vec3(0.2f,0.3f,0.0f), 1.0f);
		view->lightAttenuation = world->lightAttenuation;
		view->spotLightDir = vec4(player.Front, 0.0f);
		view->spotLightParams = vec4(
			cos(radians(world->spotLightParams.x)),
			cos(radians(world->spotLightParams.y)),
			world->spotLightParams.z,
			world->spotLightParams.w);

		view->renderView.viewID = 1;
		view->renderView.fov = radians(player.Zoom*1.5f);
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

		world->RenderView(view);
		lastNumDrawSurf = view->numDrawSurfs;


		return 0;
	}
}