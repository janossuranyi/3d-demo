#include "engine/engine.h"
#include <SDL.h>
#include <filesystem>
#include <memory>
#include "demo.h"
#include "logger.h"

#include "effect_model_loading.h"
#include "effect_test_engine.h"
#include "gpu.h"
#include "gpu_types.h"
#include "gpu_utils.h"
#include "window.h"

#define SCREEN_WIDTH 1440
#define SCREEN_HEIGHT 900
#define FULLSCREEN false

bool V_Init(int w, int h, int multisample, bool fullscreen);
void V_Shutdown();


static bool V_Init(int w, int h, int multisample, bool fullscreen)
{
    Window window{ w,h,24,32,8,multisample,8,8,8,8,fullscreen};

    return GPU::setup(window, "3d-demo");
}

static void V_Shutdown()
{
    GPU::close();
}

void App_EventLoop()
{
    SDL_Event e;
    bool running = true;
    float prev = float(SDL_GetTicks());

    std::unique_ptr<Effect> activeEffect = std::make_unique<EngineTestEffect>();

    if (!activeEffect->Init())
        return;

    int sampleCount = 0;
    Uint32 ticks = 0;
    GLsync sync{};
    uint64_t frame_num{ 0 };

    while (running)
    {
        float now = float(SDL_GetTicks());
        float time = now - prev;
        prev = now;

        running = activeEffect->Update(time);

        while (SDL_PollEvent(&e) != SDL_FALSE && running)
        {
            running = activeEffect->HandleEvent(&e, time);

            if (e.type == SDL_QUIT)
            {
                running = false;
            }
            else if(e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_ESCAPE)
                {
                    running = false;
                }
            }
        }


        Uint32 tick1 = SDL_GetTicks();
#if 0
        GL_FLUSH_ERRORS();

        if (sync) GL_CHECK(glDeleteSync(sync));
#endif
        if (!activeEffect->Render(frame_num))
        {
            running = false;
        }
#if 0
        GL_CHECK(sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));

        //glFinish();

        GL_CHECK(glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, ~GLuint64(0)));
#endif
        Uint32 tick2 = SDL_GetTicks();
        ticks += tick2 - tick1;
        if (++sampleCount > 2000)
        {
            SDL_Log("ftime: %f", (ticks / 2000.0f));
            sampleCount = 0;
            ticks = 0;
        }
#if 0
        GPU::flipSwapChain();
#endif
        ++frame_num;
    }
}

namespace fs = std::filesystem;

int main(int argc, char** argv)
{

    rc::FileSystem::set_working_dir(fs::absolute(fs::path("../")).generic_string());

    rc::ResourceManager::add_resource_path("../assets/shaders");
    rc::ResourceManager::add_resource_path("../assets/textures");

    Info("V_Init Start");
#if 0
    if (V_Init(SCREEN_WIDTH, SCREEN_HEIGHT, 0, FULLSCREEN))
    {
        Info("V_Init Done");


        App_EventLoop();
    }

    Info("V_Shutdown...");
    V_Shutdown();
#endif
    App_EventLoop();
	Info("Program terminated");
	return 0;
}