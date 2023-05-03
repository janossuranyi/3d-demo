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

    
    Engine engine;

    if ( !engine.Init( false ) )
    {
        return 0;
    }

    ModelManager MM{};
    RenderModel* rm = MM.LoadFromGLTF(resourceMgr->GetResource("scenes/sponza/Sponza.gltf"), 0);

    std::atomic_bool quit{};
    Image* im  = new Image( "imag1" );
    renderSystem.imageManager->AddImage( im );

    renderSystem.backend->SetCurrentTextureUnit( 0 );

//    std::filesystem::path p( "../assets/textures/debug_uv.dds" );
    std::filesystem::path p("d:/data/kloofendal_48d_partly_cloudy_puresky_4k.hdr");

    if ( !std::filesystem::exists( p ) || !im->Load( p.string().c_str() ) )
    {
        Error( "Cannot load texture" );
    }

    renderSystem.backend->SetCurrentTextureUnit( 1 );
    im->Bind();

    SDL_Event e;

    while ( !quit )
    {
        renderSystem.Frame();

        while ( SDL_PollEvent( &e ) != SDL_FALSE )
        {
            if ( e.type == SDL_QUIT )
            {
                quit = true;
                break;
            }
        }
        
        std::this_thread::sleep_for( 1ms );
    }

    //jsr::renderSystem.Shutdown();

    return 0;
}