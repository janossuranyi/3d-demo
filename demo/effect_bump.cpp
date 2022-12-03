#include <string>
#include "effect_bump.h"
#include "engine/gfx/renderer.h"
#include "engine/resource/context.h"
#include "engine/resource/resource_manager.h"
#include "engine/gfx/draw_vert.h"
#include <tiny_gltf.h>

bool BumpEffect::Init()
{
    auto hwr    = ctx::Context::default()->hwr();
    auto tm     = ctx::Context::default()->textureManager();
    auto vc     = ctx::Context::default()->vertexCache();
    auto sm     = ctx::Context::default()->shaderManager();

    diffuse_    = tm->createFromResource("textures/test/crashed_plane_01_slvr.tga");
    normal_     = tm->createFromResource("textures/test/crashed_plane_01_slvr_nm.tga");
    bump_       = tm->createFromResource("textures/test/crashed_plane_01_slvr_bump.tga");


    using namespace tinygltf;
    Model model;
    TinyGLTF loader;
    String err, warn;

    if (!loader.LoadASCIIFromFile(&model, &err, &warn, rc::ResourceManager::get_resource("models/plane.gltf"))) {
        Error("err: %s, warn: %s", err.c_str(), warn.c_str());
        return false;
    }

    Mesh* plane;
    Vector<gfx::DrawVert> verts;

    for (int i = 0; i < model.meshes.size(); ++i)
    {
        if (model.meshes[i].name == "Plane")
        {
            plane = &model.meshes[i];
            Primitive& p = plane->primitives[0];
            int P = p.attributes["POSITION"];
            int N = p.attributes["NORMAL"];
            int Tn = p.attributes["TANGENT"];
            int Tx = p.attributes["TEXCOORD_0"];

            auto& P_bufferView = model.bufferViews[model.accessors[P].bufferView];
            auto& P_buffer = model.buffers[P_bufferView.buffer];
            const vec3* pP = reinterpret_cast<const vec3*>(P_buffer.data.data() + P_bufferView.byteOffset + model.accessors[P].byteOffset);

            auto& N_bufferView = model.bufferViews[model.accessors[N].bufferView];
            auto& N_buffer = model.buffers[N_bufferView.buffer];
            const vec3* pN = reinterpret_cast<const vec3*>(N_buffer.data.data() + N_bufferView.byteOffset + model.accessors[N].byteOffset);

            auto& Tn_bufferView = model.bufferViews[model.accessors[Tn].bufferView];
            auto& Tn_buffer = model.buffers[Tn_bufferView.buffer];
            const vec4* pTn = reinterpret_cast<const vec4*>(Tn_buffer.data.data() + Tn_bufferView.byteOffset + model.accessors[Tn].byteOffset);

            auto& Tx_bufferView = model.bufferViews[model.accessors[Tx].bufferView];
            auto& Tx_buffer = model.buffers[Tx_bufferView.buffer];
            const vec2* pTx = reinterpret_cast<const vec2*>(Tx_buffer.data.data() + Tx_bufferView.byteOffset + model.accessors[Tx].byteOffset);

            auto& I_bufferView = model.bufferViews[model.accessors[p.indices].bufferView];
            auto& I_buffer = model.buffers[I_bufferView.buffer];
            const ushort* pI = reinterpret_cast<const ushort*>(I_buffer.data.data() + I_bufferView.byteOffset + model.accessors[p.indices].byteOffset);
            const auto vertexCount = model.accessors[P].count;
            const auto indexCount = model.accessors[p.indices].count;

            for (int v = 0; v < vertexCount; ++v)
            {
                gfx::DrawVert& vtx = verts.emplace_back();
                vtx.setPosition(pP[v]);
                vtx.setNormal(pN[v]);
                vtx.setTexCoord(pTx[v]);
                vtx.setTangent(pTn[v]);
            }

            vcache = vc->allocStaticVertex(gfx::Memory(verts));
            icache = vc->allocStaticIndex(gfx::Memory(pI, vertexCount * 2));
        }
    }

    shader_ = sm->createProgram(gfx::RenderProgram{ "pbrMetallicRoughness","shaders/pbr.vs.glsl","shaders/pbr.fs.glsl","",{} });


    return true;
}

bool BumpEffect::Update(float time)
{
    return false;
}

bool BumpEffect::HandleEvent(const SDL_Event* ev, float time)
{
    return false;
}

bool BumpEffect::Render(uint64_t frame)
{
    return false;
}
