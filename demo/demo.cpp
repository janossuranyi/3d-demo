#include <thread>
#include <chrono>
#include <atomic>
#include <filesystem>
#include <iostream>
#include <SDL.h>
#include "engine2/Engine.h"
#include "engine2/RenderSystem.h"
#include "engine2/BufferObject.h"
#include "engine2/Image.h"
#include "engine2/Logger.h"
#include "engine2/ThreadWorker.h"
#include "engine2/TaskExecutor.h"
#include "engine2/System.h"

using namespace std::chrono;

int xdata[] = { 1,2,3,4 };

void task(int* data)
{
    std::cout << std::endl << "data = " << *data << ", tid = " << std::this_thread::get_id() << std::endl;
}

int main(int argc, char** argv)
{
    using namespace std::chrono_literals;

    jsr::MessageBox(jsr::MESSAGEBOX_INFO, "Info", "JSR-Engine Demo");

    std::cout << "tid = " << std::this_thread::get_id() << std::endl;

    if (!jsr::renderSystem.Init())
    {
        return 0;
    }

    jsr::TaskExecutor t1;
    jsr::TaskExecutor t2;

    t1.Start(1);
    t2.Start(2);
    jsr::TaskExecutor* pool[]{ &t1,&t2 };

    jsr::TaskList tl(1, jsr::PRIO_LOW);
    tl.AddTask((jsr::taskfun_t)task, &xdata[0]);
    tl.AddTask((jsr::taskfun_t)task, &xdata[1]);
    tl.AddTask((jsr::taskfun_t)task, &xdata[2]);
    tl.AddTask((jsr::taskfun_t)task, &xdata[3]);

    tl.Submit(pool, 2);
    tl.Wait();
    Info("TaskList finished");

    std::atomic_bool quit{};
    jsr::Image* im = new jsr::Image("imag1");
    jsr::Image* im2 = new jsr::Image("imag2");
    jsr::renderSystem.backend->SetCurrentTextureUnit(0);

    std::filesystem::path p("../assets/textures/concrete/ConcreteWall02_2K_BaseColor_ect1s.ktx2");

    if (!std::filesystem::exists(p) || !im->Load(p.string().c_str()))
    {
        Error("Cannot load texture");
    }

    jsr::imageOpts_t opts{};
    opts.sizeX = 512;
    opts.sizeY = 512;
    opts.format = jsr::IMF_R11G11B10F;
    opts.compressed = false;
    opts.numLevel = 1;
    opts.shape = jsr::IMS_2D;
    opts.srgb = false;
    opts.usage = jsr::IMU_DIFFUSE;
    im2->AllocImage(opts, jsr::IFL_LINEAR, jsr::IMR_CLAMP_TO_EDGE);

    jsr::renderSystem.backend->SetCurrentTextureUnit(0);
    im2->Bind();
    jsr::renderSystem.backend->SetCurrentTextureUnit(1);
    im->Bind();

    SDL_Event e;

    while (!quit)
    {
        jsr::renderSystem.Frame();

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


    jsr::renderSystem.Shutdown();

    return 0;
}