#include "engine/engine.h"
#include <SDL.h>
#include <iostream>
#include <filesystem>
#include <memory>
#include <tiny_gltf.h>
#include <cmath>
#include <type_traits>
#include <tuple>
#include "demo.h"
#include "logger.h"

#include "effect_test_engine.h"
#include "effect_bump.h"
#include "gpu.h"
#include "gpu_types.h"
#include "gpu_utils.h"
#include "window.h"

#include "engine/gfx/image.h"
#include "engine/gfx/material.h"
#include "engine/gfx/draw_vert.h"
#include "engine/resource/allocator.h"

#define SCREEN_WIDTH 1440
#define SCREEN_HEIGHT 900
#define FULLSCREEN false


static void App_EventLoop()
{
    SDL_Event e;
    bool running = true;
    float prev = float(SDL_GetTicks());

    std::unique_ptr<Effect> activeEffect = std::make_unique<BumpEffect>();

    if (!activeEffect->Init())
        return;

    int sampleCount = 0;
    Uint32 ticks = 0;
    uint64_t frame_num{ 0 };

    gfx::Renderer* hwr = ctx::Context::default()->hwr();

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
        if (!activeEffect->Render(frame_num))
        {
            running = false;
        }

        hwr->frame();

        Uint32 tick2 = SDL_GetTicks();
        ticks += tick2 - tick1;
        if (++sampleCount > 2000)
        {
            SDL_Log("ftime: %f", (ticks / 2000.0f));
            sampleCount = 0;
            ticks = 0;
        }
        ++frame_num;
    }
}

void demo_1() {
namespace fs = std::filesystem;

    rc::FileSystem::set_working_dir(fs::absolute(fs::path("../")).generic_string());

    rc::ResourceManager::add_resource_path("../assets/shaders");
    rc::ResourceManager::add_resource_path("../assets/textures");
    rc::ResourceManager::add_resource_path("../assets/models");

    Info("V_Init Start");

    gfx::Renderer* hwr = ctx::Context::default()->hwr();

    if (!hwr->init(gfx::RendererType::OpenGL, 1440, 900, "test", 1))
    {
        return;
    }

    App_EventLoop();

	Info("Program terminated");

    return;
}