#include <string>
#include <tiny_gltf.h>

#include "effect_bump.h"
#include "engine/gfx/renderer.h"
#include "engine/resource/context.h"
#include "engine/resource/resource_manager.h"
#include "engine/gfx/draw_vert.h"
#include "engine/scene/camera.h"
#include <math/bounds.h>

bool BumpEffect::Init()
{
    auto hwr    = ctx::Context::default()->hwr();
    auto tm     = ctx::Context::default()->textureManager();
    auto vc     = ctx::Context::default()->vertexCache();
    auto sm     = ctx::Context::default()->shaderManager();

    bool no_srgb = false;

    String base_name = "crashed_plane_04_wrinkle";

    diffuse_ = tm->createFromResource("textures/test/" + base_name + ".tga", true, false);
    normal_ = tm->createFromResource("textures/test/" + base_name + "_nm.tga", no_srgb, false);
    bump_ = tm->createFromResource("textures/test/" + base_name + "_bump.tga", no_srgb, false);


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
                vtx.setColor(vec4(1.0f * (v % 2), 1.0f * (1 - (v % 2)), 1.0f * ((v % 3) == 1), 1.0f));

                Info("position %d: [%.2f, %.2f, %.2f]", v, pP[v].x, pP[v].y, pP[v].z);
                Info("normal   %d: [%.2f, %.2f, %.2f]", v, pN[v].x, pN[v].y, pN[v].z);
                Info("tangent  %d: [%.2f, %.2f, %.2f, %.2f]", v, pTn[v].x, pTn[v].y, pTn[v].z, pTn[v].w);
            }

            vcache = vc->allocStaticVertex(gfx::Memory(verts));
            icache = vc->allocStaticIndex(gfx::Memory(pI, indexCount * 2));
        }
    }

    shader_ = sm->createProgram(gfx::RenderProgram{ "pbrMetallicRoughness","shaders/pbr.vs.glsl","shaders/pbr.fs.glsl","",{} });

    rot_ = vec3(0, 0, 0);

    lpos_ = vec3(0.0f, 2.0f, 0.0f);
    lpower_ = 6.0f;

    SDL_SetRelativeMouseMode(SDL_TRUE);
    cam_.MouseSensitivity = 0.1;

    return true;
}

bool BumpEffect::Update(float time)
{
    //rot_.y += 0.01f * time;
    return true;
}

bool BumpEffect::HandleEvent(const SDL_Event* ev, float time)
{
    if (ev->type == SDL_MOUSEMOTION)
    {
        auto m = ev->motion;
        cam_.ProcessMouseMovement(m.xrel, -m.yrel);
    }
    if (ev->type == SDL_KEYDOWN)
    {
        auto k = ev->key.keysym.sym;
        float m = 0.05f;

        float t = time * 0.0005f;
        if (k == SDLK_w) cam_.ProcessKeyboard(scene::FORWARD, t);
        if (k == SDLK_s) cam_.ProcessKeyboard(scene::BACKWARD, t);
        if (k == SDLK_a) cam_.ProcessKeyboard(scene::LEFT, t);
        if (k == SDLK_d) cam_.ProcessKeyboard(scene::RIGHT, t);

        if (k == SDLK_1) rot_.x -= 2.f;
        if (k == SDLK_2) rot_.x += 2.f;
        if (k == SDLK_3) rot_.y -= 2.f;
        if (k == SDLK_4) rot_.y += 2.f;
        if (k == SDLK_5) rot_.z -= 2.f;
        if (k == SDLK_6) rot_.z += 2.f;

        if (k == SDLK_y) lpos_.x += m;
        if (k == SDLK_x) lpos_.x -= m;
        if (k == SDLK_c) lpos_.y += m;
        if (k == SDLK_v) lpos_.y -= m;
        if (k == SDLK_b) lpos_.z += m;
        if (k == SDLK_n) lpos_.z -= m;

        if (k == SDLK_r) lpos_ = vec3(0);
        if (k == SDLK_KP_PLUS) lpower_ += .5f;
        if (k == SDLK_KP_MINUS) lpower_ -= .5f;

        lpower_ = std::max(lpower_, 0.0f);

        Info("light pos{% .2f,% .2f,% .2f}", lpos_.x, lpos_.y, lpos_.z);
        Info("Zpos{%.2f}, lpower{%.2f}", vZ_, lpower_);
    }
    return true;
}

bool BumpEffect::Render(uint64_t frame)
{
    auto hwr = ctx::Context::default()->hwr();
    auto vc = ctx::Context::default()->vertexCache();

    uint voffset, vsize;
    uint ioffset, isize;
    gfx::VertexBufferHandle vb = vc->getVertexBuffer<gfx::DrawVert>(vcache, voffset, vsize);
    gfx::IndexBufferHandle ib = vc->getIndexBuffer<ushort>(icache, ioffset, isize);

    mat4 V(1.0f);
    mat4 W(1.0f);
    mat4 P(1.0f);
    mat3 N;
    mat4 WVP;

    vec2 screenSize = hwr->getFramebufferSize();

    V = cam_.GetViewMatrix();

    W = glm::rotate(W, glm::radians(rot_.x), vec3(1, 0, 0));
    W = glm::rotate(W, glm::radians(rot_.y), vec3(0, 1, 0));
    W = glm::rotate(W, glm::radians(rot_.z), vec3(0, 0, 1));

    P = glm::perspective(45.0f, screenSize.x / screenSize.y, 0.1f, 100.0f);
    N = glm::transpose(glm::inverse(mat3(W)));

    WVP = P * V * W;
    vec3 lightPos = lpos_;
    vec3 lightColor = vec3(1.0f) * lpower_;

    uniforms_["g_mWorldViewProjection"] = WVP;
    uniforms_["g_mWorldTransform"] = W;
    uniforms_["g_mNormalTransform"] = N;
    uniforms_["g_vViewPosition"] = vec4(cam_.Position, 1.0);
    uniforms_["g_lights[0].pos"] = lightPos;
    uniforms_["g_lights[0].color"] = lightColor;
    uniforms_["samp_basecolor"] = 0;
    uniforms_["samp_normal"] = 1;
    uniforms_["samp_pbr"] = 2;

    hwr->setClearBits(0, gfx::GLS_CLEAR_COLOR | gfx::GLS_CLEAR_DEPTH);
    hwr->setClearColor(0, vec4(0.1f, 0.1f, 0.1f, 1));
    hwr->setFrameBuffer(0, gfx::FrameBufferHandle{ 0 });

    hwr->setRenderState(gfx::GLS_DEPTHFUNC_LESS);
    hwr->setVertexBuffer(vb);
    hwr->setIndexBuffer(ib);
    hwr->setPrimitiveType(gfx::PrimitiveType::Triangles);
    hwr->setVertexDecl(*hwr->defaultVertexDecl());
    hwr->setTexture(diffuse_, 0);
    hwr->setTexture(normal_, 1);
    hwr->setTexture(bump_, 2);
    hwr->setUniforms(uniforms_);

    hwr->submit(0, shader_, isize, voffset, ioffset*2);

    W = glm::translate(mat4(1.0), lightPos);
    W = glm::scale(W, vec3(0.2));
    N = glm::transpose(glm::inverse(mat3(W)));
    WVP = P * V * W;

    uniforms_["g_mWorldViewProjection"] = WVP;
    uniforms_["g_mWorldTransform"] = W;
    uniforms_["g_mNormalTransform"] = N;
    uniforms_["g_vViewPosition"] = vec4(cam_.Position,1.0);
    uniforms_["g_lights[0].pos"] = lightPos;
    uniforms_["g_lights[0].color"] = lightColor;
    uniforms_["samp_basecolor"] = 0;
    uniforms_["samp_normal"] = 1;
    uniforms_["samp_pbr"] = 2;

    hwr->setRenderState(gfx::GLS_DEPTHFUNC_LESS);
    hwr->setVertexBuffer(vb);
    hwr->setIndexBuffer(ib);
    hwr->setPrimitiveType(gfx::PrimitiveType::Triangles);
    hwr->setVertexDecl(*hwr->defaultVertexDecl());
    hwr->setTexture(diffuse_, 0);
    hwr->setTexture(normal_, 1);
    hwr->setTexture(bump_, 2);
    hwr->setUniforms(uniforms_);

    hwr->setClearBits(1, 0);
    hwr->setFrameBuffer(1, gfx::FrameBufferHandle{ 0 });

    hwr->submit(1, shader_, isize, voffset, ioffset * 2);


    return true;
}

BumpEffect::~BumpEffect()
{
    SDL_SetRelativeMouseMode(SDL_FALSE);
}
