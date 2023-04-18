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

using namespace std::chrono;

using namespace std::chrono_literals;

int main(int argc, char** argv)
{
    Info("JS Engine Demo v0.1");
    Info("Platform: %s, PID: %d", jsr::GetPlatform(), std::this_thread::get_id());
    Info("Installed memory: %dMB", jsr::GetSystemRAM());

    jsr::Bounds box1{};
    box1 << (glm::vec3(-1, -1, -1));
    box1 << (glm::vec3(1, 1, 1));

    glm::vec3 center = box1.GetCenter();

    Info("box center [%.2f, %.2f, %.2f], radius: %.2f", center.x, center.y, center.z, box1.GetRadius());
    
    auto corners = box1.GetCorners();
    for (int i = 0; i < 8; ++i)
    {
        std::cout << corners[i].x << ", " << corners[i].y << ", " << corners[i].z << std::endl;
    }
    
    float tmin{}, tmax{};
    bool ok = box1.RayIntersect(glm::vec3(2, 0, 0), glm::vec3(-1, 0, 0), tmin, tmax);
    Info("Intersect result (%i) %.2f, %.2f", ok, tmin, tmax);

    exit(1);

    //jsr::MessageBox(jsr::MESSAGEBOX_INFO, "Info", "JSR-Engine Demo");


    if (!jsr::renderSystem.Init())
    {
        return 0;
    }

    jsr::ResourceManager::instance.AddResourcePath("../assets/textures");

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

    delete im;
    delete im2;

    //jsr::renderSystem.Shutdown();

    return 0;
}