#include <thread>
#include <chrono>
#include <atomic>
#include <filesystem>
#include <SDL.h>
#include "engine2/Engine.h"
#include "engine2/RenderSystem.h"
#include "engine2/BufferObject.h"
#include "engine2/Image.h"
#include "engine2/Logger.h"
#include "engine2/ThreadWorker.h"

using namespace std::chrono;

class XWorker : public jsr::ThreadWorker
{
public:
    XWorker(int x) { this->X = x; this->Y = 0; }
    int GetX() const { return X; }
    int GetY() const { return Y; }
    int Run() override;
private:
    int X;
    int Y;
};

int XWorker::Run()
{
    std::this_thread::sleep_for(milliseconds(X));
    Y += X;

    return 0;
}

int main(int argc, char** argv)
{
    using namespace std::chrono_literals;

    if (!jsr::renderSystem.Init())
    {
        return 0;
    }

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

    XWorker wk(20);
    wk.StartWorkerThread("yuppi");

    int y = wk.GetY();
    while (!quit)
    {
        if (y != wk.GetY())
        {
            y = wk.GetY();
            if ((y % 1000) == 0) {
                Info("Y = %d", y);
            }
        }


        jsr::renderSystem.Frame();
        wk.SignalWork();

        while (SDL_PollEvent(&e) != SDL_FALSE)
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
                break;
            }
        }
        //wk.WaitForThread();
    }


    jsr::renderSystem.Shutdown();

    return 0;
}