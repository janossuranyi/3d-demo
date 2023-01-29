#include "JSE.h"
#include "JSE_GfxCoreGL46.h"
#include <nv_dds.h>
#include <ktx.h>
#include <gl_format.h>

#define SCREEN_WIDTH 14401
#define SCREEN_HEIGHT 900
#define FULLSCREEN false


void demo_1();

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
    static void getAttribs(JseVertexInputAttributeDescription* p) {
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
    case 0x8E8D: return JseFormat::RGBA_BPTC;
    case 0x8E8E: return JseFormat::RGBA_BPTC;
    default:
        return JseFormat::RGBA8;
    }
}

KTX_error_code imageCB(int miplevel, int face, int width, int height, int depth, ktx_uint64_t faceLodSize, void* pixels, void* userdata)
{
    JseGfxRenderer* R = RCAST(JseGfxRenderer*, userdata);
    JseCmdUploadImage& c = *R->GetCommandBuffer<JseCmdUploadImage>();

    c.info.data = R->FrameAlloc<uint8_t>(faceLodSize);
    std::memcpy(c.info.data, pixels, faceLodSize);

    c.info.imageId = JseImageID{ 1 };
    c.info.face = face;
    c.info.width = width;
    c.info.height = height;
    c.info.level = miplevel;
    c.info.depth = 1;
    c.info.imageSize = faceLodSize;

    Info("level %d, face %d, width %d, height %d, depth %d, size %d", miplevel, face, width, height, depth, faceLodSize);

    return KTX_SUCCESS;
}

int main(int argc, char** argv)
{
    using namespace nv_dds;

    JseInit(argc, argv);

    bool running = true;

    const char* vtxshader{ R"(
#version 450

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_TexCoord;

out vec2 vofi_TexCoord;

uniform vec4 _va_freqHigh[4];

#define g_WorldMatrix_X _va_freqHigh[0]
#define g_WorldMatrix_Y _va_freqHigh[1]
#define g_WorldMatrix_Z _va_freqHigh[2]
#define g_WorldMatrix_W _va_freqHigh[3]

void main() {
    mat4 W = mat4(
        g_WorldMatrix_X,
        g_WorldMatrix_Y,
        g_WorldMatrix_Z,
        g_WorldMatrix_W
    );

    gl_Position = W * vec4(in_Position, 1.0);
    vofi_TexCoord = in_TexCoord;
}
)" };

    const char* fragshader{ R"(
#version 450

in vec2 vofi_TexCoord;

layout(binding = 0) uniform sampler2D samp0;

out vec4 fragColor;

void main() {
    fragColor = texture(samp0, vofi_TexCoord.xy);
}
)" };

    namespace fs = std::filesystem;
    JseFileSystem::set_working_dir(fs::absolute(fs::path("../")).generic_string());

    JseResourceManager::add_resource_path("../assets/shaders");
    JseResourceManager::add_resource_path("../assets/textures");
    JseResourceManager::add_resource_path("../assets/models");

    try {
        JseInputManager I;
        JseGfxRenderer R;

        I.SetOnExitEvent([&]
            {
                running = false;
            });

        I.SetOnKeyboardEvent([&](JseKeyboardEvent e)
            {
                Info("Key event: %x:%04x", e.type, e.keysym.sym);
                if (e.keysym.sym == JseKeyCode::SDLK_ESCAPE) {
                    running = false;
                }
            });

        R.SetCore(new JseGfxCoreGL());
        R.InitCore(1024, 768, false, true);

        JseCmdCreateShader& sp1 = *R.GetCommandBuffer<JseCmdCreateShader>();
        sp1.info.pCode = vtxshader;
        sp1.info.shaderId = JseShaderID{ 1 };
        sp1.info.stage = JseShaderStage::VERTEX;
        JseCmdCreateShader& sp2 = *R.GetCommandBuffer<JseCmdCreateShader>();
        sp2.info.pCode = fragshader;
        sp2.info.shaderId = JseShaderID{ 2 };
        sp2.info.stage = JseShaderStage::FRAGMENT;

        JseCmdCreateDescriptorSetLayoutBindind& setBinding = *R.GetCommandBuffer<JseCmdCreateDescriptorSetLayoutBindind>();
        setBinding.info.setLayoutId = JseDescriptorSetLayoutID{ 1 };
        setBinding.info.bindingCount = 1;
        setBinding.info.pBindings = R.FrameAlloc<JseDescriptorSetLayoutBinding>(1);
        setBinding.info.pBindings[0].binding = 0;
        setBinding.info.pBindings[0].descriptorCount = 1;
        setBinding.info.pBindings[0].descriptorType = JseDescriptorType::SAMPLED_IMAGE;
        setBinding.info.pBindings[0].stageFlags = JSE_STAGE_FLAG_FRAGMENT;


        JseCmdCreateDescriptorSetLayoutBindind& setBinding2 = *R.GetCommandBuffer<JseCmdCreateDescriptorSetLayoutBindind>();
        setBinding2.info.setLayoutId = JseDescriptorSetLayoutID{ 2 };
        setBinding2.info.bindingCount = 1;
        setBinding2.info.pBindings = R.FrameAlloc<JseDescriptorSetLayoutBinding>(1);
        setBinding2.info.pBindings[0].binding = 100;
        setBinding2.info.pBindings[0].descriptorCount = 1;
        setBinding2.info.pBindings[0].descriptorType = JseDescriptorType::INLINE_UNIFORM_BLOCK;
        setBinding2.info.pBindings[0].stageFlags = JSE_STAGE_FLAG_ALL;

        JseCmdCreateGraphicsPipeline& p = *R.GetCommandBuffer<JseCmdCreateGraphicsPipeline>();
        p.info.graphicsPipelineId = JseGrapicsPipelineID{ 1 };
        p.info.renderState = GLS_DEPTHFUNC_LESS;
        p.info.setLayoutId = JseDescriptorSetLayoutID{ 1 };
        p.info.stageCount = 2;
        p.info.pStages = R.FrameAlloc<JsePipelineShaderStageCreateInfo>(2);
        p.info.pStages[0].stage = JseShaderStage::VERTEX;
        p.info.pStages[0].shader = JseShaderID{ 1 };
        p.info.pStages[1].stage = JseShaderStage::FRAGMENT;
        p.info.pStages[1].shader = JseShaderID{ 2 };

        p.info.pVertexInputState = R.FrameAlloc<JsePipelineVertexInputStateCreateInfo>();
        p.info.pVertexInputState->attributeCount = xVertex::attributeCount();
        p.info.pVertexInputState->pAttributes = R.FrameAlloc<JseVertexInputAttributeDescription>(xVertex::attributeCount());
        p.info.pVertexInputState->bindingCount = xVertex::bindingCount();
        p.info.pVertexInputState->pBindings = R.FrameAlloc<JseVertexInputBindingDescription>(xVertex::bindingCount());
        xVertex::getBindings(p.info.pVertexInputState->pBindings);
        xVertex::getAttribs(p.info.pVertexInputState->pAttributes);

        JseCmdCreateBuffer& buf = *R.GetCommandBuffer<JseCmdCreateBuffer>();
        buf.info.bufferId = JseBufferID{ 1 };
        buf.info.size = 64 * 1024;
        //buf.info.storageFlags = JSE_BUFFER_STORAGE_DYNAMIC_BIT|JSE_BUFFER_STORAGE_COHERENT_BIT|JSE_BUFFER_STORAGE_PERSISTENT_BIT|JSE_BUFFER_STORAGE_WRITE_BIT;
        buf.info.target = JseBufferTarget::VERTEX;

        JseCmdUpdateBuffer& upd = *R.GetCommandBuffer<JseCmdUpdateBuffer>();
        upd.info.bufferId = JseBufferID{ 1 };
        upd.info.size = sizeof(rect);
        upd.info.data = (uint8_t*)&rect[0];


        R.Frame();

        {
            ktxTexture* kTexture;
            KTX_error_code ktxresult;
            ktxresult = ktxTexture_CreateFromNamedFile(
                JseResourceManager::get_resource("textures/test/HerringboneMarbleTiles01_2048.ktx2").c_str(),
//                JseResourceManager::get_resource("textures/cubemaps/skybox.ktx2").c_str(),
                KTX_TEXTURE_CREATE_NO_FLAGS,
                &kTexture);

            if (ktxresult != KTX_SUCCESS) {
                exit(255);
            }

            assert(kTexture->classId == ktxTexture2_c);
            ktxTexture2* kt2 = (ktxTexture2*)kTexture;
            Info("isCompressed: %d", kt2->isCompressed);

            JseImageTarget target{JseImageTarget::D2};

            if (kt2->isCubemap) {
                target = JseImageTarget::CUBEMAP;
            }
            else if (kt2->numDimensions == 1) {
                target = JseImageTarget::D1;
            }
            else if (kt2->numDimensions == 3) {
                target = JseImageTarget::D3;
            }

            auto& img = *R.GetCommandBuffer<JseCmdCreateImage>();
            img.info.imageId = JseImageID{ 1 };
            img.info.format = JseFormat::RGBA_BPTC;
            img.info.target = target;
            img.info.height = kt2->baseHeight;
            img.info.width = kt2->baseWidth;
            img.info.depth = kt2->baseDepth;
            img.info.levelCount = kt2->numLevels;
            img.info.srgb = kt2->vkFormat == VK_FORMAT_BC7_SRGB_BLOCK;
            img.info.compressed = kt2->isCompressed;
            img.info.immutable = 1;

            R.GetCore()->SetVSyncInterval(0);
            ktxTexture_IterateLoadLevelFaces(kTexture, imageCB, &R);
            R.GetCore()->SetVSyncInterval(1);
        }

        JseCmdBindGraphicsPipeline& bind = *R.GetCommandBuffer<JseCmdBindGraphicsPipeline>();
        bind.pipeline = JseGrapicsPipelineID{ 1 };

        JseCmdBindVertexBuffers& bb = *R.GetCommandBuffer<JseCmdBindVertexBuffers>();
        bb.bindingCount = 1;
        bb.firstBinding = 0;
        bb.pBuffers = R.FrameAlloc<JseBufferID>(1);
        bb.pBuffers[0] = JseBufferID{ 1 };
        bb.pOffsets = R.FrameAlloc<JseDeviceSize>(1);
        bb.pOffsets[0] = 0;
        bb.pipeline = JseGrapicsPipelineID{ 1 };

        auto& crdset = *R.GetCommandBuffer<JseCmdCreateDescriptorSet>();
        crdset.info.setId = JseDescriptorSetID{ 1 };
        crdset.info.setLayoutId = JseDescriptorSetLayoutID{ 1 };
        auto& crdset2 = *R.GetCommandBuffer<JseCmdCreateDescriptorSet>();
        crdset2.info.setId = JseDescriptorSetID{ 2 };
        crdset2.info.setLayoutId = JseDescriptorSetLayoutID{ 2 };

        auto& wrdset = *R.GetCommandBuffer<JseCmdWriteDescriptorSet>();
        wrdset.info.descriptorCount = 1;
        wrdset.info.descriptorType = JseDescriptorType::SAMPLED_IMAGE;
        wrdset.info.dstBinding = 0;
        wrdset.info.setId = JseDescriptorSetID{ 1 };
        wrdset.info.pImageInfo = R.FrameAlloc<JseDescriptorImageInfo>();
        wrdset.info.pImageInfo[0].image = JseImageID{ 1 };


        auto& bindset = *R.GetCommandBuffer<JseCmdBindDescriptorSets>();
        bindset.descriptorSetCount = 2;
        bindset.firstSet = 0;
        bindset.pDescriptorSets = R.FrameAlloc<JseDescriptorSetID>(2);
        bindset.pDescriptorSets[0] = JseDescriptorSetID{ 1 };
        bindset.pDescriptorSets[1] = JseDescriptorSetID{ 2 };

        R.Frame();

        float tick = SCAST(float, SDL_GetTicks64()) / 1000;
        float angle = 0.f;

        while(running) {

            JseRect2D screen{};
            R.GetCore()->GetSurfaceDimension(screen);

            JseCmdBeginRenderpass& x = *R.GetCommandBuffer<JseCmdBeginRenderpass>();
            x.info.colorClearEnable = true;
            x.info.colorClearValue.color = JseColor4f{ 0.1f,0.0f,0.1f,1.0f };
            x.info.scissorEnable = false;
            x.info.framebuffer = JseFrameBufferID{ 0 };
            x.info.viewport = screen;
            x.info.scissor.w = 512;
            x.info.scissor.h = 512;

            auto& wrdset = *R.GetCommandBuffer<JseCmdWriteDescriptorSet>();

            wrdset.info.descriptorCount = 1;
            wrdset.info.descriptorType = JseDescriptorType::INLINE_UNIFORM_BLOCK;
            wrdset.info.dstBinding = 100;
            wrdset.info.setId = JseDescriptorSetID{ 2 };
            wrdset.info.pUniformInfo = R.FrameAlloc<JseDescriptorUniformInfo>();

            glm::mat4 W(1.0f);
            W = glm::rotate(W, glm::radians(angle/2.f), glm::vec3(0, 0, 1));
            W = glm::scale(W, glm::vec3(2.f + 0.5f * std::sinf(glm::radians(angle))));
            std::strncpy(wrdset.info.pUniformInfo[0].name, "_va_freqHigh", 32);
            wrdset.info.pUniformInfo[0].vectorCount = 4;
            wrdset.info.pUniformInfo[0].pVectors = R.FrameAlloc<glm::vec4>(4);
            std::memcpy(wrdset.info.pUniformInfo[0].pVectors, &W[0][0], sizeof(W));

            auto& bindset = *R.GetCommandBuffer<JseCmdBindDescriptorSets>();
            bindset.descriptorSetCount = 1;
            bindset.firstSet = 0;
            bindset.pDescriptorSets = R.FrameAlloc<JseDescriptorSetID>(1);
            bindset.pDescriptorSets[0] = JseDescriptorSetID{ 2 };

            JseCmdDraw& draw = *R.GetCommandBuffer<JseCmdDraw>();
            draw.firstInstance = 0;
            draw.firstVertex = 0;
            draw.instanceCount = 1;
            draw.mode = JseTopology::Triangles;
            draw.vertexCount = 6;

            R.Frame();

            I.ProcessEvents();

            float now = SCAST(float, SDL_GetTicks64()) / 1000.f;
            float dt = now - tick;
            tick = now;

            angle += 30.f * dt;

        }
    }
    catch (std::exception e) { Error("error=%s", e.what()); }
    
    JseShutdown();

	Info("Program terminated");

	return 0;
}