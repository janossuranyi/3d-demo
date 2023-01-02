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

    String base_name[] = {
        "ScratchedGold01_2048",
        "PaintedMetal02_2048",
        "HerringboneMarbleTiles01_2048",
        "WoodenPlanks01_2048"
    };
    
    int bn = 1;

    diffuse_ = tm->createFromResource("textures/test/" + base_name[bn] + ".ktx2");
    normal_ = tm->createFromResource("textures/test/" + base_name[bn] + "_nm.ktx2");
    bump_ = tm->createFromResource("textures/test/" + base_name[bn] + "_bump.ktx2");
    skybox_ = tm->createFromResource("textures/cubemaps/yokohama3.ktx2");

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

    shader_ = sm->createProgram(gfx::RenderProgram{ 
        "pbrMetallicRoughness",
        "shaders/pbr2.vs.glsl",
        "shaders/pbr2.fs.glsl","",{} });

    rot_ = vec3(0, 0, 0);
    lpower_ = 0.3f;
    numLights_ = 3  ;
    float radius = 3.0f;
    float step = 360.0f / numLights_;
    float angle = 0.0f;
    vec4 colors[]{ vec4(1),vec4(1),vec4(1,0.5f,0,1) };

    for (int i = 0; i < numLights_; ++i)
    {
        Light& L = lightInfo_.g_lights[i];
        L.pos.w = 1.0f;
        L.pos.y = 2.0f;
        L.pos.x = glm::sin(glm::radians(angle))* radius;
        L.pos.z = glm::cos(glm::radians(angle)) * radius;
        L.color = colors[i];
        angle += step;
    }

    lightInfoBuffer_ = hwr->createConstantBuffer(
        sizeof(LightInfoBlock),
        gfx::BufferUsage::Dynamic,
        gfx::Memory(&lightInfo_.g_lights[0], sizeof(Light) * numLights_));

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

        const float rotSpeed = .5f;
        if (k == SDLK_1) rot_.x -= rotSpeed;
        if (k == SDLK_2) rot_.x += rotSpeed;
        if (k == SDLK_3) rot_.y -= rotSpeed;
        if (k == SDLK_4) rot_.y += rotSpeed;
        if (k == SDLK_5) rot_.z -= rotSpeed;
        if (k == SDLK_6) rot_.z += rotSpeed;

        if (k == SDLK_y) lpos_.x += m;
        if (k == SDLK_x) lpos_.x -= m;
        if (k == SDLK_c) lpos_.y += m;
        if (k == SDLK_v) lpos_.y -= m;
        if (k == SDLK_b) lpos_.z += m;
        if (k == SDLK_n) lpos_.z -= m;

        if (k == SDLK_r) lpos_ = vec3(0);
        if (k == SDLK_KP_PLUS) lpower_ += time * .01f;
        if (k == SDLK_KP_MINUS) lpower_ -= time * .01f;
        if (k == SDLK_k) geoKoefFlag_ = 1 - geoKoefFlag_;
        lpower_ = std::max(lpower_, 0.0f);
    }
    return true;
}

bool BumpEffect::Render(uint64_t frame)
{    

    Vector<vec4> univec;

    auto hwr = ctx::Context::default()->hwr();
    auto vc = ctx::Context::default()->vertexCache();

    uint voffset, vsize;
    uint ioffset, isize;
    gfx::VertexBufferHandle vb = vc->getVertexBuffer<gfx::DrawVert>(vcache, voffset, vsize);
    gfx::IndexBufferHandle ib = vc->getIndexBuffer<ushort>(icache, ioffset, isize);

    hwr->setConstBuffer(0, lightInfoBuffer_, 0, sizeof(Light)*3);

    uniforms_["samp_basecolor"] = 0;
    uniforms_["samp_normal"] = 1;
    uniforms_["samp_pbr"] = 2;
    uniforms_["samp_env"] = 3;
    firstframe_ = false;


    mat4 V(1.0f);
    mat4 W(1.0f);
    mat4 P(1.0f);
    mat3 N;
    mat4 WVP;

    ivec2 screenSize = hwr->getFramebufferSize();

    freqLow_vertexUniforms_.vieworigin = vec4(cam_.Position, 1.0);
    freqLow_fragmentUniforms_.lightpower = vec4(lpower_);
    freqLow_fragmentUniforms_.lightoffset = vec4(lpos_, 1.0f);
    freqLow_fragmentUniforms_.numlights.x = float(numLights_);
    freqLow_fragmentUniforms_.vieworigin = vec4(cam_.Position, 1.0);

    toVec4vec(univec, freqLow_vertexUniforms_);
    uniforms_["_va_freqLow"] = std::move(univec);
    toVec4vec(univec, freqLow_fragmentUniforms_);
    uniforms_["_fa_freqLow"] = std::move(univec);

    V = cam_.GetViewMatrix();

    W = glm::rotate(W, glm::radians(rot_.x), vec3(1, 0, 0));
    W = glm::rotate(W, glm::radians(rot_.y), vec3(0, 1, 0));
    W = glm::rotate(W, glm::radians(rot_.z), vec3(0, 0, 1));

    P = glm::perspective(glm::radians(75.0f), (float)screenSize.x / screenSize.y, 0.1f, 100.0f);
    N = glm::transpose(glm::inverse(mat3(W)));
    //N = mat3(W);

    WVP = P * V * W;

    freqHigh_vertexUniforms_.mvpmatrix = WVP;
    freqHigh_vertexUniforms_.normalmatrix = N;

    toVec4vec(univec, freqHigh_vertexUniforms_);
    uniforms_["_va_freqHigh"] = std::move(univec);

    hwr->setClearBits(0, gfx::GLS_CLEAR_COLOR | gfx::GLS_CLEAR_DEPTH);
    hwr->setClearColor(0, vec4(0.4f, 0.4f, 0.4f, 1));
    hwr->setFrameBuffer(0, gfx::FrameBufferHandle{ 0 });
    hwr->setViewport(0, 0, 0, screenSize.x, screenSize.y);

    hwr->setRenderState(gfx::GLS_DEPTHFUNC_LESS);
    hwr->setVertexBuffer(vb);
    hwr->setIndexBuffer(ib);
    hwr->setPrimitiveType(gfx::PrimitiveType::Triangles);
    hwr->setVertexDecl(*hwr->defaultVertexDecl());
    hwr->setTexture(diffuse_, 0);
    hwr->setTexture(normal_, 1);
    hwr->setTexture(bump_, 2);
    hwr->setTexture(skybox_, 3);
    hwr->setUniforms(uniforms_);

    hwr->submit(0, shader_, isize, voffset, ioffset*2);

    hwr->setViewport(1, 0, 0, screenSize.x, screenSize.y);

    for (int l = 0; l < numLights_; ++l)
    {

        W = glm::translate(mat4(1.0), vec3(lightInfo_.g_lights[l].pos) + lpos_);
        W = glm::scale(W, vec3(0.2));
        N = glm::transpose(glm::inverse(mat3(W)));
        WVP = P * V * W;

        freqHigh_vertexUniforms_.mvpmatrix = WVP;
        freqHigh_vertexUniforms_.normalmatrix = N;

        toVec4vec(univec, freqHigh_vertexUniforms_);
        uniforms_["_va_freqHigh"] = std::move(univec);

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
    }

    return true;
}

BumpEffect::~BumpEffect()
{
    SDL_SetRelativeMouseMode(SDL_FALSE);
}
