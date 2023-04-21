#include <thread>
#include <chrono>
#include <atomic>
#include <filesystem>
#include <iostream>
#include <fstream>

#include <SDL.h>
#include "engine2/Engine.h"
#include "engine2/RenderSystem.h"
#include "engine2/BufferObject.h"
#include "engine2/Image.h"
#include "engine2/Logger.h"
#include "engine2/ThreadWorker.h"
#include "engine2/TaskExecutor.h"
#include "engine2/System.h"
#include "engine2/Resources.h"
#include "engine2/Bounds.h"
#include "engine2/RenderProgs.h"
#include "engine2/Engine.h"

using namespace std::chrono;

using namespace std::chrono_literals;
using namespace glm;
using namespace jsr;

int main(int argc, char** argv)
{
    Info("JS Engine Demo v0.1");
    Info("Platform: %s, PID: %d", jsr::GetPlatform(), std::this_thread::get_id());
    Info("Installed memory: %dMB", jsr::GetSystemRAM());

    resourceMgr->AddResourcePath("../assets/shaders");
    resourceMgr->AddResourcePath("../assets/textures");

    //jsr::MessageBox(jsr::MESSAGEBOX_INFO, "Info", "JSR-Engine Demo");

    Engine engine;

    if (!engine.Init(false))
    {
        return 0;
    }
    

    std::atomic_bool quit{};
    Image* im = new jsr::Image("imag1");
    Image* im2 = new jsr::Image("imag2");
    renderSystem.backend->SetCurrentTextureUnit(0);

    std::filesystem::path p("../assets/textures/concrete/ConcreteWall02_2K_BaseColor_ect1s.ktx2");

    if (!std::filesystem::exists(p) || !im->Load(p.string().c_str()))
    {
        Error("Cannot load texture");
    }

    imageOpts_t opts{};
    opts.sizeX = 512;
    opts.sizeY = 512;
    opts.format = IMF_R11G11B10F;
    opts.compressed = false;
    opts.numLevel = 1;
    opts.shape = IMS_2D;
    opts.srgb = false;
    opts.usage = IMU_DIFFUSE;
    im2->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE);

    renderSystem.backend->SetCurrentTextureUnit(0);
    im2->Bind();
    renderSystem.backend->SetCurrentTextureUnit(1);
    im->Bind();

    SDL_Event e;

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
        }
        
        std::this_thread::sleep_for(1ms);
    }

    delete im;
    delete im2;

    //jsr::renderSystem.Shutdown();

    return 0;
}