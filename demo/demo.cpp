#include "engine/engine.h"
#include <SDL.h>
#include <iostream>
#include <filesystem>
#include <memory>
#include <tiny_gltf.h>
#include <cmath>
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

#define SCREEN_WIDTH 1440
#define SCREEN_HEIGHT 900
#define FULLSCREEN false


void App_EventLoop()
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

namespace fs = std::filesystem;

void traverse_node(tinygltf::Model const& model, int node, int level = 0)
{
    std::string s{};
    for (int i = 0; i < level; ++i) s += "\t";

    tinygltf::Node const& N = model.nodes[node];

    {
        Info("--------------------------------------------------------------------------");
        Info("%s Node %s", s.c_str(), N.name.c_str());
        if (N.matrix.size())
        {
            Info("%s matrix", s.c_str());
        }
        if (N.rotation.size())
        {
            Info("%s rotation (%f,%f,%f,%f)", s.c_str(), N.rotation[0], N.rotation[1], N.rotation[2], N.rotation[3]);
        }
        if (N.scale.size())
        {
            Info("%s scale (%f,%f,%f)", s.c_str(), N.scale[0], N.scale[1], N.scale[2]);
        }
        if (N.translation.size())
        {
            Info("%s translation (%f,%f,%f)", s.c_str(), N.translation[0], N.translation[1], N.translation[2]);
        }
    }

    if (N.mesh >= 0)
    {
        tinygltf::Mesh const& M = model.meshes[N.mesh];
        Info("%s Mesh %d %s pr:%d", s.c_str(), N.mesh, M.name.c_str(),M.primitives.size());
    }
    else if (N.camera >= 0)
    {
        tinygltf::Camera const& C = model.cameras[N.camera];
        Info("%s Camera %d %s fov:%f", s.c_str(), N.camera, C.name.c_str(), glm::degrees( C.perspective.yfov ));
    }
    else if (N.extensions.size())
    {
        for (auto const& ext : N.extensions)
        {
            Info("%s Ext: %s", s.c_str(), ext.first.c_str());
            if (ext.first == "KHR_lights_punctual")
            {
                int light = ext.second.Get("light").GetNumberAsInt();
                auto const& L = model.lights[light];            
                Info("%s Light %d, Typ:%s", s.c_str(), light, L.type.c_str());
            }
        }
    }

    for (int child : model.nodes[node].children)
    {
        traverse_node(model, child, level + 1);
    }
}

int main(int argc, char** argv)
{

    rc::FileSystem::set_working_dir(fs::absolute(fs::path("../")).generic_string());

    rc::ResourceManager::add_resource_path("../assets/shaders");
    rc::ResourceManager::add_resource_path("../assets/textures");
    rc::ResourceManager::add_resource_path("../assets/models");

    using namespace tinygltf;
    using namespace std;

    TinyGLTF loader;
    Model model;
    string err, warn;

    Info("sizeof(Model)=%d", sizeof(Model));

    auto file = rc::ResourceManager::get_resource("models/scene.gltf");
    if (loader.LoadASCIIFromFile(&model, &err, &warn, file))
    {
        Info("Version: %s, %s", model.asset.version.c_str(), model.asset.generator.c_str());
        for (auto& e : model.extensionsRequired) {
            Info("%s", e.c_str());
        }
    }

    for (int node : model.scenes[model.defaultScene].nodes)
    {
        traverse_node(model, node, 0);
    }


    Info("V_Init Start");

    gfx::Renderer* hwr = ctx::Context::default()->hwr();

    if (!hwr->init(gfx::RendererType::OpenGL, 1440, 900, "test", 1))
    {
        return false;
    }

    App_EventLoop();

	Info("Program terminated");

	return 0;
}