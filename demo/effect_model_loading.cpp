#include <string>
#include "gpu_buffer.h"
#include "pipeline.h"
#include "filesystem.h"
#include "demo.h"

#include "effect_model_loading.h"

bool LoadModelEffect::Init()
{
    Mesh3D model;

    std::string fname = g_fileSystem.resolve("assets/Steampunk_Dirigible_with_Ship.glb");
    model.loadFromGLTF(fname.c_str(), 0, 0);
    m_mesh.compile(model);

    pipeline.setScreenRect(0, 0, videoConf.width, videoConf.height);
    pipeline.update(0.0f);

    return true;
}

bool LoadModelEffect::Update(float time)
{
    return false;
}

bool LoadModelEffect::HandleEvent(const SDL_Event* ev)
{
    return false;
}

void LoadModelEffect::Render()
{
}
