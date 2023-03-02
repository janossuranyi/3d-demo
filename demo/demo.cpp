#include "JSE.h"
#include "JSE_GfxCoreGL46.h"
#include <nv_dds.h>
#include <ktx.h>
#include <gl_format.h>
#include <chrono>

#define SCREEN_WIDTH 1440
#define SCREEN_HEIGHT 900
#define FULLSCREEN false

using namespace glm;
using namespace js;

struct UniformMatrixes {
    alignas(16) mat4 W;
    alignas(16) mat4 V;
    alignas(16) mat4 P;
    alignas(16) mat4 WVP;
};

UniformMatrixes* uniformMatrixes{};

static const int ON_FLIGHT_FRAME = 2;

struct xVertex {
    glm::vec3 pos;
    glm::vec2 uv;

    static uint32_t bindingCount() { return 1; }
    static uint32_t attributeCount() { return 2; }
    static void getBindings(JseVertexInputBindingDescription* p) {
        p->binding = 0;
        p->inputRate = JseVertexInputRate::VERTEX;
        p->stride = 3 * 4 + 2 * 4;
    }
    static void getAttributes(JseVertexInputAttributeDescription* p) {
        p[0].bindig = 0;
        p[0].location = 0;
        p[0].format = JseFormat::RGB32F;
        p[0].offset = 0;
        p[1].bindig = 0;
        p[1].location = 1;
        p[1].format = JseFormat::RG32F;
        p[1].offset = 12;
    }
};

const xVertex rect[]{
    {{-0.5f,   0.5f, 0.0f},{0.0f, 1.0f}},
    {{ 0.5f,  -0.5f, 0.0f},{1.0f, 0.0f}},
    {{ 0.5f,   0.5f, 0.0f},{1.0f, 1.0f}},
    {{ 0.5f,  -0.5f, 0.0f},{1.0f, 0.0f}},
    {{-0.5f,   0.5f, 0.0f},{0.0f, 1.0f}},
    {{-0.5f,  -0.5f, 0.0f},{0.0f, 0.0f}}
};

JseFormat MapGLCompressedFmt(uint32_t f) {
    switch (f)
    {
    case 0x83F0: return JseFormat::RGB_DXT1;
    case 0x83F1: return JseFormat::RGBA_DXT1;
    case 0x83F2: return JseFormat::RGBA_DXT3;
    case 0x83F3: return JseFormat::RGBA_DXT5;
    case 0x8E8D:
    case 0x8E8E: return JseFormat::RGBA_BPTC;
    default:
        return JseFormat::RGBA8;
    }
}

vec3 gamma(vec3 c) {
    return pow(c, vec3(1.0f / 2.2f));
}

vec4 gamma(vec4 c) {
    return vec4( pow(vec3(c), vec3(1.0f / 2.2f)), c.a );
}

static const char* vtxshader{ R"(
#version 450

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_TexCoord;

out INTERFACE { 
    vec2 TexCoord;
} Out;

layout(binding = 1, std140) uniform UniformMatrixes {
    mat4 W;
    mat4 V;
    mat4 P;
    mat4 WVP;
};

void main() {

    gl_Position = WVP * vec4(in_Position, 1.0);
    Out.TexCoord = in_TexCoord;
}
)" };

static const char* fragshader{ R"(
#version 450

in INTERFACE { 
    vec2 TexCoord;
} In;

layout(binding = 0) uniform sampler2D samp0;
layout(binding = 1) uniform sampler2D samp1;
layout(binding = 2) uniform sampler2D samp2;

out vec4 fragColor;

vec3 gamma(vec3 c) {
    return pow(c, vec3(1.0/2.2));
}
vec4 gamma(vec4 c) {
    return vec4(pow(c.rgb, vec3(1.0/2.2)), c.a);
}

void main() {
    
    vec4 c0 = texture(samp0, In.TexCoord.xy);
    
    fragColor = vec4( c0.xyz, 1.0 );
}
)" };


struct JseFormat_t {
    JseFormat fmt;
    bool srgb;
    //JseFormat_t() = default;
};

static const JsHashMap<ktx_uint32_t, JseFormat_t> s_vkf2jse_map {
    {VK_FORMAT_BC1_RGB_UNORM_BLOCK,         {JseFormat::RGB_DXT1,    false}},
    {VK_FORMAT_BC1_RGBA_UNORM_BLOCK,        {JseFormat::RGBA_DXT1,   false}},
    {VK_FORMAT_BC1_RGB_SRGB_BLOCK,          {JseFormat::RGB_DXT1,    true}},
    {VK_FORMAT_BC1_RGBA_SRGB_BLOCK,         {JseFormat::RGBA_DXT1,   true}},
    {VK_FORMAT_BC2_UNORM_BLOCK,             {JseFormat::RGBA_DXT3,   false}},
    {VK_FORMAT_BC2_SRGB_BLOCK,              {JseFormat::RGBA_DXT3,   true}},
    {VK_FORMAT_BC3_UNORM_BLOCK,             {JseFormat::RGBA_DXT5,   false}},
    {VK_FORMAT_BC3_SRGB_BLOCK,              {JseFormat::RGBA_DXT5,   true}},
    {VK_FORMAT_BC7_UNORM_BLOCK,             {JseFormat::RGBA_BPTC,   false}},
    {VK_FORMAT_BC7_SRGB_BLOCK,              {JseFormat::RGBA_BPTC,   true}}
};

class Demo :public Engine {
public:
    Demo() = default;
    virtual bool Init() override;
    virtual void Run() override;
    //virtual void Done() override {};
    struct RenderContext {

        JseBufferID buf_Vertex;
        JseBufferID buf_UbMatrix;
        JseDescriptorSetLayoutID layout_Img_Unibuf;
        JseDescriptorSetLayoutID layout_Uniform;
        JseGrapicsPipelineID pipeline;
        JseShaderID shader_vertex;
        JseShaderID shader_fragment;
        JseImageID texture;
        JseDescriptorSetID set_texture;
        JseDescriptorSetID set_uniform;
        int frame;
    } ctx;
private:

    bool running = true;
    float dt{};
    bool t{};
    float tick;
    float angle;
    float time1;
    JseCmdBeginRenderpass renderPass{};
    vec3 viewOrigin{ 0.f,0.f,-2.f };
    mat4 W{ 1.0f }, V, P;
    UniformMatrixes ub_mtx{};

    JseFenceID syncId[ON_FLIGHT_FRAME]{};
    uint8_t* ptr{};
};

KTX_error_code imageCB(int miplevel, int face, int width, int height, int depth, ktx_uint64_t faceLodSize, void* pixels, void* userdata)
{
    auto* D = reinterpret_cast<Demo*>(userdata);
    auto* R = D->GetRenderer();
    JseCmdUploadImage c = JseCmdUploadImage();

    //c.info.data = R->FrameAlloc<uint8_t>(faceLodSize);
    //std::memcpy(c.info.data, pixels, faceLodSize);

    c.info.data = (uint8_t*)pixels;
    c.info.imageId = D->ctx.texture;
    c.info.face = face;
    c.info.width = width;
    c.info.height = height;
    c.info.level = miplevel;
    c.info.depth = 1;
    c.info.imageSize = faceLodSize;

    Info("level %d, face %d, width %d, height %d, depth %d, size %d", miplevel, face, width, height, depth, faceLodSize);
    R->UploadImage(c.info);

    return KTX_SUCCESS;
}

bool Demo::Init() {
    namespace fs = std::filesystem;
    FileSystem::set_working_dir(fs::absolute(fs::path("../")).generic_string());

    Filesystem::add_resource_path("../assets/shaders");
    Filesystem::add_resource_path("../assets/textures");
    Filesystem::add_resource_path("../assets/models");
    Filesystem::add_resource_path("../assets/buildings");

    GetInputManager()->SetOnExitEvent([&] { running = false; });
    GetInputManager()->SetOnKeyboardEvent([&](JseKeyboardEvent e)
        {
            //Info("Key event: %x:%04x", e.type, e.keysym.sym);
            if (e.keysym.sym == JseKeyCode::SDLK_ESCAPE) {
                running = false;
            }

            if (e.state == JSE_RELEASED) {
                if (e.keysym.sym == JseKeyCode::SDLK_t) {
                    Info("dt: %.2f ms", 1000 * dt);
                }
            }
        });

    GetRenderer()->SetVSyncInterval(0);
    /***********************************************************/
    /* Creating shaders                                        */
    /***********************************************************/
    GfxRenderer& R = *GetRenderer();
    ctx.shader_vertex = R.CreateShader();
    ctx.shader_fragment = R.CreateShader();

    auto& wr1 = *R.GetCommandBuffer();
    auto& wr2 = *R.GetCommandBuffer();
    JseCmdCreateShader sp1{};
    sp1.info.pCode = vtxshader;
    sp1.info.shaderId = ctx.shader_vertex;
    sp1.info.stage = JseShaderStage::VERTEX;
    JseCmdCreateShader sp2{};
    sp2.info.pCode = fragshader;
    sp2.info.shaderId = ctx.shader_fragment;
    sp2.info.stage = JseShaderStage::FRAGMENT;
    wr1.command = sp1;
    wr2.command = sp2;

    /***********************************************************/
    /* Creating descriptorset layout */
    /***********************************************************/

    ctx.layout_Img_Unibuf = JseDescriptorSetLayoutID{ R.NextID() };

    auto& setBinding = *R.CreateCommand<JseCmdCreateDescriptorSetLayoutBindind>();
    setBinding.info.setLayoutId = ctx.layout_Img_Unibuf;
    setBinding.info.bindingCount = 1;
    setBinding.info.pBindings = R.FrameAlloc<JseDescriptorSetLayoutBinding>(1);
    setBinding.info.pBindings[0].binding = 0;
    setBinding.info.pBindings[0].descriptorCount = 1;
    setBinding.info.pBindings[0].descriptorType = JseDescriptorType::SAMPLED_IMAGE;
    setBinding.info.pBindings[0].stageFlags = JSE_STAGE_FLAG_FRAGMENT;

    ctx.layout_Uniform = JseDescriptorSetLayoutID{ R.NextID() };
    auto& setBinding2 = *R.CreateCommand<JseCmdCreateDescriptorSetLayoutBindind>();
    setBinding2.info.setLayoutId = ctx.layout_Uniform;
    setBinding2.info.bindingCount = 1;
    setBinding2.info.pBindings = R.FrameAlloc<JseDescriptorSetLayoutBinding>(setBinding2.info.bindingCount);
    setBinding2.info.pBindings[0].binding = 1;
    setBinding2.info.pBindings[0].descriptorCount = 1;
    setBinding2.info.pBindings[0].descriptorType = JseDescriptorType::UNIFORM_BUFFER_DYNAMIC;
    setBinding2.info.pBindings[0].stageFlags = JSE_STAGE_FLAG_VERTEX;

    /***********************************************************/
    /* Creating graphics pipeline
    /***********************************************************/
    auto& p = *R.CreateCommand<JseCmdCreateGraphicsPipeline>();
    ctx.pipeline = JseGrapicsPipelineID{ R.NextID() };
    p.info.graphicsPipelineId = ctx.pipeline;
    p.info.renderState = GLS_DEPTHFUNC_LESS;
    p.info.setLayoutId = JseDescriptorSetLayoutID{};
    p.info.stageCount = 2;
    p.info.pStages = R.FrameAlloc<JsePipelineShaderStageCreateInfo>(2);
    p.info.pStages[0].stage = JseShaderStage::VERTEX;
    p.info.pStages[0].shader = ctx.shader_vertex;
    p.info.pStages[1].stage = JseShaderStage::FRAGMENT;
    p.info.pStages[1].shader = ctx.shader_fragment;

    p.info.pVertexInputState = R.FrameAlloc<JsePipelineVertexInputStateCreateInfo>();
    p.info.pVertexInputState->attributeCount = xVertex::attributeCount();
    p.info.pVertexInputState->pAttributes = R.FrameAlloc<JseVertexInputAttributeDescription>(xVertex::attributeCount());
    p.info.pVertexInputState->bindingCount = xVertex::bindingCount();
    p.info.pVertexInputState->pBindings = R.FrameAlloc<JseVertexInputBindingDescription>(xVertex::bindingCount());
    xVertex::getBindings(p.info.pVertexInputState->pBindings);
    xVertex::getAttributes(p.info.pVertexInputState->pAttributes);

    /***********************************************************/
    /* Creating buffers */
    /***********************************************************/

    ctx.buf_Vertex = R.CreateBuffer();
    ctx.buf_UbMatrix = R.CreateBuffer();
    auto& buf = *R.CreateCommand<JseCmdCreateBuffer>();
    buf.info.bufferId = ctx.buf_Vertex;
    buf.info.size = 64 * 1024;
    buf.info.target = JseBufferTarget::VERTEX;

    auto& ubuf = *R.CreateCommand<JseCmdCreateBuffer>();
    ubuf.info.bufferId = ctx.buf_UbMatrix;
    ubuf.info.size = 64 * 1024;
    ubuf.info.target = JseBufferTarget::UNIFORM_DYNAMIC;
    ubuf.info.storageFlags = JSE_BUFFER_STORAGE_COHERENT_BIT | JSE_BUFFER_STORAGE_PERSISTENT_BIT | JSE_BUFFER_STORAGE_WRITE_BIT;

    auto& upd = *R.CreateCommand<JseCmdUpdateBuffer>();
    upd.info.bufferId = ctx.buf_Vertex;
    upd.info.size = sizeof(rect);
    upd.info.data = (uint8_t*)&rect[0];

    R.Frame();
    ptr = SCAST(uint8_t*, R.GetMappedBufferPointer(ctx.buf_UbMatrix));


    /***********************************************************/
    /* Creating textures */
    /***********************************************************/

    {
        ktxTexture* kTexture;
        KTX_error_code ktxresult;
        bool tex_not_loaded = true;
        ktxresult = ktxTexture_CreateFromNamedFile(
            Filesystem::get_resource("textures/concrete/ConcreteWall02_2K_BaseColor_ect1s.ktx2").c_str(),
            //                JseResourceManager::get_resource("textures/cubemaps/skybox.ktx2").c_str(),
            KTX_TEXTURE_CREATE_NO_FLAGS,
            &kTexture);

        if (ktxresult != KTX_SUCCESS) {
            exit(255);
        }

        assert(kTexture->classId == ktxTexture2_c);
        ktxTexture2* kt2 = (ktxTexture2*)kTexture;
        Info("isCompressed: %d", kt2->isCompressed);
        if (ktxTexture2_NeedsTranscoding(kt2)) {
            ktx_texture_transcode_fmt_e tf = KTX_TTF_BC1_OR_3;

            const auto start = std::chrono::steady_clock::now();

            ktxresult = ktxTexture2_TranscodeBasis(kt2, tf, 0);

            const auto end = std::chrono::steady_clock::now();
            const auto diff = end - start;

            using namespace std::literals;
            Info("Transcode took %d us", (end - start) / 1us);

            // Then use VkUpload or GLUpload to create a texture object on the GPU.
            if (ktxresult != KTX_SUCCESS) {
                exit(255);
            }
            tex_not_loaded = false;
        }


        auto& fmt = s_vkf2jse_map.find(kt2->vkFormat);

        if (fmt == s_vkf2jse_map.end()) {
            Info("Unknown texture format !");
            exit(255);
        }

        JseImageTarget target{ JseImageTarget::D2 };

        if (kt2->isCubemap) {
            target = JseImageTarget::CUBEMAP;
        }
        else if (kt2->numDimensions == 1) {
            target = JseImageTarget::D1;
        }
        else if (kt2->numDimensions == 3) {
            target = JseImageTarget::D3;
        }

        auto& img = *R.CreateCommand<JseCmdCreateImage>();
        ctx.texture = R.CreateImage();
        img.info.imageId = ctx.texture;
        img.info.format = fmt->second.fmt;
        img.info.target = target;
        img.info.height = kt2->baseHeight;
        img.info.width = kt2->baseWidth;
        img.info.depth = kt2->baseDepth;
        img.info.levelCount = kt2->numLevels;
        img.info.srgb = fmt->second.srgb;
        img.info.compressed = kt2->isCompressed;
        img.info.immutable = 1;
        img.info.samplerDescription = R.FrameAlloc<JseSamplerDescription>();
        img.info.samplerDescription->minFilter = JseFilter::LINEAR_MIPMAP_LINEAR;
        img.info.samplerDescription->magFilter = JseFilter::LINEAR;
        img.info.samplerDescription->lodBias = 0.f;
        img.info.samplerDescription->maxAnisotropy = 1.0f;
        img.info.samplerDescription->maxLod = SCAST(float, kt2->numLevels);
        img.info.samplerDescription->tilingS = JseImageTiling::CLAMP_TO_BORDER;
        img.info.samplerDescription->tilingT = JseImageTiling::CLAMP_TO_BORDER;
        img.info.samplerDescription->tilingR = JseImageTiling::CLAMP_TO_BORDER;

        R.Frame();

        if (tex_not_loaded) {
            ktxresult = ktxTexture_IterateLoadLevelFaces(kTexture, imageCB, this);
        }
        else {
            ktxresult = ktxTexture_IterateLevelFaces(kTexture, imageCB, this);
        }

        ktxTexture_Destroy(kTexture);

        if (ktxresult != KTX_SUCCESS) {
            Info("Cannot load texture");
            exit(255);
        }
    }


    auto& bind = *R.CreateCommand<JseCmdBindGraphicsPipeline>();
    bind.pipeline = ctx.pipeline;

    auto& bb = *R.CreateCommand<JseCmdBindVertexBuffers>();
    bb.bindingCount = 1;
    bb.firstBinding = 0;
    bb.pBuffers = R.FrameAlloc<JseBufferID>(1);
    bb.pBuffers[0] = ctx.buf_Vertex;
    bb.pOffsets = R.FrameAlloc<JseDeviceSize>(1);
    bb.pOffsets[0] = 0;
    bb.pipeline = ctx.pipeline;

    ctx.set_texture = JseDescriptorSetID(R.NextID());
    auto& crdset = *R.CreateCommand<JseCmdCreateDescriptorSet>();
    crdset.info.setId = ctx.set_texture;
    crdset.info.setLayoutId = ctx.layout_Img_Unibuf;

    ctx.set_uniform = JseDescriptorSetID(R.NextID());
    auto& crdset2 = *R.CreateCommand<JseCmdCreateDescriptorSet>();
    crdset2.info.setId = ctx.set_uniform;
    crdset2.info.setLayoutId = ctx.layout_Uniform;

    auto* wrdset = R.CreateCommand<JseCmdWriteDescriptorSet>();
    wrdset->info.descriptorCount = 1;
    wrdset->info.descriptorType = JseDescriptorType::SAMPLED_IMAGE;
    wrdset->info.dstBinding = 0;
    wrdset->info.setId = ctx.set_texture;
    wrdset->info.pImageInfo = R.FrameAlloc<JseDescriptorImageInfo>();
    wrdset->info.pImageInfo[0].image = ctx.texture;

    auto& bindset = *R.CreateCommand<JseCmdBindDescriptorSets>();
    bindset.descriptorSetCount = 2;
    bindset.firstSet = 0;
    bindset.pDescriptorSets = R.FrameAlloc<JseDescriptorSetID>(2);
    bindset.pDescriptorSets[0] = ctx.set_texture;
    bindset.pDescriptorSets[1] = ctx.set_uniform;

    auto* write1 = R.CreateCommand<JseCmdWriteDescriptorSet>();
    write1->info.setId = ctx.set_uniform;
    write1->info.descriptorCount = 1;
    write1->info.descriptorType = JseDescriptorType::UNIFORM_BUFFER_DYNAMIC;
    write1->info.dstBinding = 1;
    write1->info.pBufferInfo = R.FrameAlloc<JseDescriptorBufferInfo>();
    write1->info.pBufferInfo[0].buffer = ctx.buf_UbMatrix;
    write1->info.pBufferInfo[0].size = sizeof(UniformMatrixes);

    R.Frame();


    ivec2 screen{};
    R.core()->GetSurfaceDimension(screen);
    auto scissor_w = screen.x / 2;
    auto scissor_h = screen.y / 2;

    renderPass.info.colorClearEnable = true;
    renderPass.info.colorClearValue.color = glm::vec4{ 0.1f,0.0f,0.1f,1.0f };
    renderPass.info.depthClearEnable = true;
    renderPass.info.depthClearValue.depth = 1.0f;
    renderPass.info.scissorEnable = false;
    renderPass.info.framebuffer = JseFrameBufferID{ 0 };
    renderPass.info.viewport = JseRect2D{ 0,0,screen.x,screen.y };
    renderPass.info.scissor.x = (screen.x - scissor_w) / 2;
    renderPass.info.scissor.y = (screen.y - scissor_h) / 2;
    renderPass.info.scissor.w = scissor_w;
    renderPass.info.scissor.h = scissor_h;

    R.SetVSyncInterval(1);

    tick = SCAST(float, SDL_GetTicks64()) / 1000.f;
    angle = 0.f;
    time1 = tick;


    P = perspective(radians(75.0f), (float)screen.x / screen.y, 0.01f, 100.f);


    return true;
}

void Demo::Run() {
    GfxRenderer& R = *GetRenderer();

    while (running) {

        R.SubmitCommand(renderPass);

        viewOrigin.z = 0.2f + (2.0f + 2.0f * std::sinf(radians(angle)));

        {

            W = mat4(1.0f);
            //W = rotate(W, radians(angle), vec3(0.f, 1.f, 0.f));
            W = rotate(W, radians(angle), vec3(0.f, 0.f, 1.f));
            V = lookAt(viewOrigin, vec3{ 0.f,0.f,0.f }, vec3{ 0.f,1.f,0.f });
            if (syncId[ctx.frame]) {
                R.WaitSync(syncId[ctx.frame], 1000 * 1000);
            }

            auto* uf = RCAST(UniformMatrixes*, ptr + ctx.frame * 256);
            uf->P = P;
            uf->V = V;
            uf->W = W;
            uf->WVP = P * V * W;

            auto* bindset = R.CreateCommand<JseCmdBindDescriptorSets>();
            bindset->descriptorSetCount = 1;
            bindset->firstSet = 0;
            bindset->pDescriptorSets = R.FrameAlloc<JseDescriptorSetID>(1);
            bindset->pDescriptorSets[0] = ctx.set_uniform;
            bindset->dynamicOffsetCount = 1;
            bindset->pDynamicOffsets = R.FrameAlloc<uint32_t>();
            bindset->pDynamicOffsets[0] = ctx.frame * 256;

            auto* draw = R.CreateCommand<JseCmdDraw>();
            draw->instanceCount = 1;
            draw->mode = JseTopology::Triangles;
            draw->vertexCount = 6;

            if (syncId[ctx.frame]) {
                R.CreateCommand<JseDeleteFenceCmd>()->id = syncId[ctx.frame];
            }
            else {
                syncId[ctx.frame] = JseFenceID{ R.NextID() };
            }
            R.CreateCommand<JseCreateFenceCmd>()->id = syncId[ctx.frame];
        }

        R.Frame();

        GetInputManager()->ProcessEvents();

        ctx.frame = (ctx.frame + 1) % ON_FLIGHT_FRAME;

        float now = static_cast<float>(SDL_GetTicks64()) / 1000.f;
        dt = now - tick;
        tick = now;

        angle += 15.f * dt;
        angle = std::fmodf(angle, 360.f);
    }
}

int main(int argc, char** argv)
{
    using namespace nv_dds;

    JseInit(argc, argv);

    try {
        auto demo = std::make_unique<Demo>();
        demo->Init();
        demo->Run();
        demo->Done();
    }
    catch (std::exception e) { Error("error=%s", e.what()); }

    JseShutdown();
    
    Info("Program terminated");

    return 0;
}