#include <SDL.h>
#include <GL/glew.h>
#include <string>
#include <map>
#include <memory>
#include "demo.h"
#include "logger.h"
#include "filesystem.h"
#include "effect_pointcube.h"
#include "effect_compute_test.h"
#include "gpu_types.h"
#include "gpu_utils.h"
#include "mesh.h"

#define SCREEN_WIDTH 1440
#define SCREEN_HEIGHT 900
#define FULLSCREEN false

VideoConfig videoConf;

bool V_Init(int w, int h, int multisample, bool fullscreen);
void V_Shutdown();

const char* BASE_DIR = "d:/src/3d-demo-git/";

static bool V_Init(int w, int h, int multisample, bool fullscreen)
{
    int err;

    if ((err = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0)) {
        Error("ERROR: %s", SDL_GetError());
        return false;
    }

    Info("SD_Init done");

    /*
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, SDL_TRUE);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    //SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, SDL_TRUE);

    if (multisample > 0)
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, multisample);
    }

    Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

    if (fullscreen && w == -1 && h == -1)
    {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    else if (fullscreen)
    {
        flags |= SDL_WINDOW_FULLSCREEN;
    }

    Info("SD_CreateWindow start");
    videoConf.hWindow = SDL_CreateWindow("3d-demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, flags);
    if (!videoConf.hWindow)
    {
        Error("Cannot create window - %s", SDL_GetError());
        return false;
    }

    Info("SD_CreateWindow done");

    videoConf.hGl = SDL_GL_CreateContext(videoConf.hWindow);
    if (!videoConf.hGl)
    {
        Error("Cannot create GL context - %s", SDL_GetError());
        return false;
    }

    Info("SDL_GL_CreateContext done");

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        Error("Cannot initialize GLEW");
        return false;
    }

    Info("glewInit done");

    SDL_GL_SetSwapInterval(1);

    std::string renderer = (char *)glGetString(GL_RENDERER);
    std::string version  = (char *)glGetString(GL_VERSION);
    const float gl_version = float(atof(version.c_str()));
    videoConf.glVersion = gl_version * 100;

    if (videoConf.glVersion < 450)
    {
        Error("Sorry, I need at least OpenGL 4.5");
        return false;
    }

    SDL_version ver;

    SDL_GetVersion(&ver);

    Info("GL Renderer: %s", renderer.c_str());
    Info("GL Version: %s (%.2f)", version.c_str(), gl_version);
    Info("SDL version: %d.%d patch %d", (int)ver.major, (int)ver.minor, (int)ver.patch);

    int _w, _h;

    SDL_GetWindowSize(videoConf.hWindow, &_w, &_h);
    glViewport(0, 0, _w, _h);
    glScissor(0, 0, _w, _h);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

#ifdef _DEBUG
    if (GLEW_ARB_debug_output)
    {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);

        glDebugMessageCallbackARB(&DebugMessageCallback, NULL);
        //glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);
        //glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);

    }
#endif
    if (GLEW_ARB_explicit_uniform_location)
    {
        videoConf.explicitUnifromLocationEXT = true;
    }

    SDL_GL_GetAttribute(SDL_GL_RED_SIZE,    &videoConf.redBits);
    SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE,  &videoConf.greenBits);
    SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE,   &videoConf.blueBits);
    SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE,  &videoConf.depthBits);
    SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &videoConf.stencilBits);


    videoConf.width = _w;
    videoConf.height = _h;

	return true;
}

static void V_Shutdown()
{
    if (videoConf.hGl)
    {
        SDL_GL_DeleteContext(videoConf.hGl);
    }
    if (videoConf.hWindow)
    {
        SDL_DestroyWindow(videoConf.hWindow);
    }
}

void App_EventLoop()
{
    SDL_Event e;
    bool running = true;
    float prev = float(SDL_GetTicks());

    std::unique_ptr<Effect> activeEffect = std::make_unique<ComputeTestEffect>();

    if (!activeEffect->Init())
        return;

    int sampleCount = 0;
    Uint32 ticks = 0;
    GLsync sync{};

    while (running)
    {
        float now = SDL_GetTicks();
        float time = now - prev;
        prev = now;

        running = activeEffect->Update(time);

        while (SDL_PollEvent(&e) != SDL_FALSE && running)
        {
            running = activeEffect->HandleEvent(&e);

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

        GL_FLUSH_ERRORS

        if (sync) GL_CHECK(glDeleteSync(sync));

        activeEffect->Render();

        GL_CHECK(sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));

        //glFinish();

        GL_CHECK(glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, ~GLuint64(0)));

        Uint32 tick2 = SDL_GetTicks();
        ticks += tick2 - tick1;
        if (++sampleCount > 200)
        {
            //SDL_Log("ftime: %f", (ticks / 200.0f));
            sampleCount = 0;
            ticks = 0;
        }


        SDL_GL_SwapWindow(videoConf.hWindow);

    }
}

int main(int argc, char** argv)
{
    g_fileSystem.set_working_dir(BASE_DIR);

//    Mesh3D mesh;
//    mesh.loadFromGLTF(g_fileSystem.resolve("assets/cube.gltf").c_str(), 0, 0);


    Info("V_Init Start");

    if (V_Init(SCREEN_WIDTH, SCREEN_HEIGHT, 0, FULLSCREEN))
    {
        Info("V_Init Done");
        App_EventLoop();
    }

    Info("V_Shutdown...");
    V_Shutdown();

	Info("Program terminated");
	return 0;
}