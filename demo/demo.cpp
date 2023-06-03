#include <thread>
#include <chrono>
#include <atomic>
#include <filesystem>
#include <iostream>
#include <fstream>


#include "engine2/Engine.h"
#include "engine2/RenderSystem.h"
#include "engine2/Resources.h"
#include "engine2/Logger.h"

using namespace jsr;

int main(int argc, char** argv)
{

    ResourceManager& rm = ResourceManager::instance;

    rm.AddResourcePath("../assets/shaders"); 
    rm.AddResourcePath("../assets/textures");
    rm.AddResourcePath("../assets/scenes");
    rm.AddResourcePath("../assets/models");

    //jsr::MessageBox(jsr::MESSAGEBOX_INFO, "Info", "JSR-Engine Demo");

    engineConfig.r_resX = 1800;
    engineConfig.r_resY = 1000;
    engineConfig.r_fullscreen = 0;

    engineConfig.r_fbsrgb = false;
#if 1
    engineConfig.r_debug = true;
#else
    engineConfig.r_debug = false;
#endif
    Engine engine;

    if ( !engine.Init( true ) )
    {
        return 0;
    }

    renderSystem.vertexCache->ClearStaticCache();
    
//
//
    //engine.LoadWorld("models/CesiumDrone.glb");
    engine.LoadWorld("scenes/sponza/sponza_j2.gltf");

    renderSystem.vertexCache->PrintStatistic();
    engine.MainLoop();


    engine.Shutdown();

    //jsr::renderSystem.Shutdown();

    return 0;
}