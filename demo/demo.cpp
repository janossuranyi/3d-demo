#include <thread>
#include <chrono>
#include <atomic>
#include <filesystem>
#include <iostream>
#include <fstream>

#include <SDL.h>
#include "engine2/Math.h"
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
#include "engine2/Node3D.h"
#include "engine2/Model.h"
#include "engine2/RenderWorld.h"
#include "engine2/ImageManager.h"

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
    resourceMgr->AddResourcePath("../assets/scenes");

    //jsr::MessageBox(jsr::MESSAGEBOX_INFO, "Info", "JSR-Engine Demo");

    engineConfig.r_fbsrgb = false;
#ifdef _DEBUG
    engineConfig.r_debug = true;
#else
    engineConfig.r_debug = false;
#endif
    Engine engine;

    if ( !engine.Init( false ) )
    {
        return 0;
    }

    renderSystem.vertexCache->ClearStaticCache();
    
    engine.LoadWorld("scenes/sponza/sponza_j.gltf");
    
    renderSystem.vertexCache->PrintStatistic();
    engine.MainLoop();


    engine.Shutdown();

    //jsr::renderSystem.Shutdown();

    return 0;
}