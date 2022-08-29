#include <string>
#include "gpu_buffer.h"
#include "pipeline.h"
#include "filesystem.h"
#include "demo.h"

#include "effect_model_loading.h"

bool LoadModelEffect::Init()
{

    if (!world.loadWorld(g_fileSystem.resolve(worldFile)))
    {
        Warning("World %s cannot load", worldFile.c_str());
    }

    pipeline.setScreenRect(0, 0, videoConf.width, videoConf.height);
    pipeline.setState( GLS_DEPTHFUNC_LESS|GLS_CULL_FRONTSIDED/* | GLS_POLYMODE_LINE*/);
    pipeline.setWorldPosition(vec3(0, 0, 0));
    pipeline.setWorldScale(vec3(1, 1, 1));
    pipeline.setWorldEulerRotation(vec3(0, 0, 0));
    pipeline.setPerspectiveCamera(glm::radians(45.0f), 0.01f, 100.0f, 0.0f);
    pipeline.setView(vec3(0, 2, 3), vec3(0, 1, 0));
    pipeline.setClearColor(.6f, .6f, 1.0f, 1.0f);

    if (!shader.loadShader(
        g_fileSystem.resolve("assets/shaders/model_test.vs.glsl"),
        g_fileSystem.resolve("assets/shaders/model_test.fs.glsl")))
    {
        return false;
    }

    //shader.bindUniformBlock("cb_matrix", 1);
    shader.use();

    return true;
}

bool LoadModelEffect::Update(float time)
{
    angleY += 0.01 * time;
    angleY = std::fmodf(angleY, 360.0f);
    return true;
}

bool LoadModelEffect::HandleEvent(const SDL_Event* ev)
{
    return true;
}

void LoadModelEffect::Render()
{

    pipeline.clear(true, true, false);

    pipeline.setWorldEulerRotation(vec3(glm::radians(-15.0f), glm::radians(angleY), 0));
    //pipeline.update();
    world.renderWorld(pipeline);

}
