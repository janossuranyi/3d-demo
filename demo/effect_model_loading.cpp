#include <string>
#include "gpu_buffer.h"
#include "pipeline.h"
#include "filesystem.h"

#include "effect_model_loading.h"

bool LoadModelEffect::Init()
{
    Mesh3D model;

    std::string fname = g_fileSystem.resolve("assets/cube.gltf");
    model.loadFromGLTF(fname.c_str(), 0, 0);
    m_mesh.compile(model);

    

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
