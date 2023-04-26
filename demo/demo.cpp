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

    //jsr::MessageBox(jsr::MESSAGEBOX_INFO, "Info", "JSR-Engine Demo");

    engineConfig.r_fbsrgb = false;


    Node3D n1{};
    Node3D n2{};

    n1.SetParent(&n2);
    n2.SetDir(glm::angleAxis(radians(90.0f), vec3(1.0f, 0.0f, 0.0f)));
    n1.SetScale(vec3(1.0f));

    auto q1 = n1.GetDir();
    Info("q1{% .2f, % .2f, % .2f, % .2f }", q1.w, q1.x, q1.y, q1.z);
    n2.SetOrigin(vec3(0.0f, 0.0f, 0.0f));
    n1.SetOrigin(vec3(0.0f, 1.0f, 0.0f));

    auto W = n1.GetLocalToWorldMatrix();

    Info("--------------------------------------------");
    for (int i = 0; i < 4; ++i)
    {
        Info("M%d{ % .2f, % .2f, % .2f, % .2f }", i, W[0][i], W[1][i], W[2][i], W[3][i]);
    }

    //exit(0);

    Engine engine;

    if ( !engine.Init( false ) )
    {
        return 0;
    }

    std::atomic_bool quit{};
    Image* im  = new Image( "imag1" );
    renderSystem.imageManager->AddImage( im );

    renderSystem.backend->SetCurrentTextureUnit( 0 );

    std::filesystem::path p( "../assets/textures/debug_uv.dds" );

    if ( !std::filesystem::exists( p ) || !im->Load( p.string().c_str() ) )
    {
        Error( "Cannot load texture" );
    }

    renderSystem.backend->SetCurrentTextureUnit( 1 );
    im->Bind();

    Image* im2 = renderSystem.imageManager->GetImage("_hdrimage");
    renderSystem.backend->SetCurrentTextureUnit( 0 );
    im2->Bind();

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