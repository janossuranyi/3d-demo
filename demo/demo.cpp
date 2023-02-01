#include "JSE.h"
#include "JSE_GfxCoreGL46.h"
#include <nv_dds.h>
#include <ktx.h>
#include <gl_format.h>

#define SCREEN_WIDTH 14401
#define SCREEN_HEIGHT 900
#define FULLSCREEN false

using namespace glm;

void demo_1();

struct UniformMatrixes {
    mat4 W;
    mat4 V;
    mat4 P;
};

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

glm::vec3 gamma(vec3 c) {
    return pow(c, vec3(1.0f / 2.2f));
}

vec4 gamma(vec4 c) {
    return vec4( pow(vec3(c), vec3(1.0f / 2.2f)), c.a );
}

KTX_error_code imageCB(int miplevel, int face, int width, int height, int depth, ktx_uint64_t faceLodSize, void* pixels, void* userdata)
{
    JseGfxRenderer* R = RCAST(JseGfxRenderer*, userdata);
    
    JseCmdUploadImage c = JseCmdUploadImage();

    //c.info.data = R->FrameAlloc<uint8_t>(faceLodSize);
    //std::memcpy(c.info.data, pixels, faceLodSize);

    c.info.data = (uint8_t*)pixels;
    c.info.imageId = JseImageID{ 1 };
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

layout(binding = 1, std140) uniform UniformMatrixes {
    mat4 W;
    mat4 V;
    mat4 P;
} matrix;

uniform mat4 g_W;

void main() {

    gl_Position = g_W * vec4(in_Position, 1.0);
    vofi_TexCoord = in_TexCoord;
}
)" };

    const char* fragshader{ R"(
#version 450

in vec2 vofi_TexCoord;

layout(binding = 0) uniform sampler2D samp0;

out vec4 fragColor;

vec3 gamma(vec3 c) {
    return pow(c, vec3(1.0/2.2));
}
vec4 gamma(vec4 c) {
    return vec4(pow(c.rgb, vec3(1.0/2.2)), c.a);
}

float alpha = 0.05;
vec3 blend = vec3(0.0, 0.0, 1.0);

void main() {
    vec3 color = texture(samp0, vofi_TexCoord.xy).xyz;
    fragColor = vec4( gamma( blend*alpha + color*(1.0-alpha) ), 1.0);
}
)" };

    namespace fs = std::filesystem;
    JseFileSystem::set_working_dir(fs::absolute(fs::path("../")).generic_string());

    JseResourceManager::add_resource_path("../assets/shaders");
    JseResourceManager::add_resource_path("../assets/textures");
    JseResourceManager::add_resource_path("../assets/models");

    float dt{};
    bool t{};

    try {
        JseInputManager I;
        JseGfxRenderer R;


        I.SetOnExitEvent([&]
            {
                running = false;
            });

        I.SetOnKeyboardEvent([&](JseKeyboardEvent e)
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

        R.SetCore(new JseGfxCoreGL());
        R.InitCore(1024, 768, false, true);

        R.SetVSyncInterval(0);

        /***********************************************************/
        /* Creating shaders                                        */
        /***********************************************************/

        auto vert = JseShaderID(R.NextID());
        auto frag = JseShaderID(R.NextID());

        auto& wr1 = *R.GetCommandBuffer();
        auto& wr2 = *R.GetCommandBuffer();
        JseCmdCreateShader sp1{};
        sp1.info.pCode = vtxshader;
        sp1.info.shaderId = vert;
        sp1.info.stage = JseShaderStage::VERTEX;
        JseCmdCreateShader sp2{};
        sp2.info.pCode = fragshader;
        sp2.info.shaderId = frag;
        sp2.info.stage = JseShaderStage::FRAGMENT;
        wr1.command = sp1;
        wr2.command = sp2;

        /***********************************************************/
        /* Creating descriptorset layout */
        /***********************************************************/

        auto texLayout = JseDescriptorSetLayoutID{ R.NextID() };

        auto& setBinding = *R.CreateCommand<JseCmdCreateDescriptorSetLayoutBindind>();
        setBinding.info.setLayoutId = texLayout;
        setBinding.info.bindingCount = 2;
        setBinding.info.pBindings = R.FrameAlloc<JseDescriptorSetLayoutBinding>(2);
        setBinding.info.pBindings[0].binding = 0;
        setBinding.info.pBindings[0].descriptorCount = 1;
        setBinding.info.pBindings[0].descriptorType = JseDescriptorType::SAMPLED_IMAGE;
        setBinding.info.pBindings[0].stageFlags = JSE_STAGE_FLAG_FRAGMENT;
        setBinding.info.pBindings[1].binding = 1;
        setBinding.info.pBindings[1].descriptorCount = 1;
        setBinding.info.pBindings[1].descriptorType = JseDescriptorType::UNIFORM_BUFFER_DYNAMIC;
        setBinding.info.pBindings[1].stageFlags = JSE_STAGE_FLAG_VERTEX;

        auto uniformLayout = JseDescriptorSetLayoutID{ R.NextID() };
        auto& setBinding2 = *R.CreateCommand<JseCmdCreateDescriptorSetLayoutBindind>();
        setBinding2.info.setLayoutId = uniformLayout;
        setBinding2.info.bindingCount = 1;
        setBinding2.info.pBindings = R.FrameAlloc<JseDescriptorSetLayoutBinding>(setBinding2.info.bindingCount);
        setBinding2.info.pBindings[0].binding = 100;
        setBinding2.info.pBindings[0].descriptorCount = 1;
        setBinding2.info.pBindings[0].descriptorType = JseDescriptorType::INLINE_UNIFORM_BLOCK;
        setBinding2.info.pBindings[0].stageFlags = JSE_STAGE_FLAG_ALL;

        /***********************************************************/
        /* Creating graphics pipeline
        /***********************************************************/
        auto& p = *R.CreateCommand<JseCmdCreateGraphicsPipeline>();
        p.info.graphicsPipelineId = JseGrapicsPipelineID{ 1 };
        p.info.renderState = GLS_DEPTHFUNC_LESS;
        p.info.setLayoutId = JseDescriptorSetLayoutID{};
        p.info.stageCount = 2;
        p.info.pStages = R.FrameAlloc<JsePipelineShaderStageCreateInfo>(2);
        p.info.pStages[0].stage = JseShaderStage::VERTEX;
        p.info.pStages[0].shader = vert;
        p.info.pStages[1].stage = JseShaderStage::FRAGMENT;
        p.info.pStages[1].shader = frag;

        p.info.pVertexInputState = R.FrameAlloc<JsePipelineVertexInputStateCreateInfo>();
        p.info.pVertexInputState->attributeCount = xVertex::attributeCount();
        p.info.pVertexInputState->pAttributes = R.FrameAlloc<JseVertexInputAttributeDescription>(xVertex::attributeCount());
        p.info.pVertexInputState->bindingCount = xVertex::bindingCount();
        p.info.pVertexInputState->pBindings = R.FrameAlloc<JseVertexInputBindingDescription>(xVertex::bindingCount());
        xVertex::getBindings(p.info.pVertexInputState->pBindings);
        xVertex::getAttribs(p.info.pVertexInputState->pAttributes);

        /***********************************************************/
        /* Creating buffers */
        /***********************************************************/

        auto& buf = *R.CreateCommand<JseCmdCreateBuffer>();
        buf.info.bufferId = JseBufferID{ 1 };
        buf.info.size = 64 * 1024;
        buf.info.target = JseBufferTarget::VERTEX;

        auto& ubuf = *R.CreateCommand<JseCmdCreateBuffer>();
        ubuf.info.bufferId = JseBufferID{ 2 };
        ubuf.info.size = 64 * 1024;
        ubuf.info.target = JseBufferTarget::UNIFORM_DYNAMIC;

        auto& upd = *R.CreateCommand<JseCmdUpdateBuffer>();
        upd.info.bufferId = JseBufferID{ 1 };
        upd.info.size = sizeof(rect);
        upd.info.data = (uint8_t*)&rect[0];

        R.Frame();

        /***********************************************************/
        /* Creating textures */
        /***********************************************************/

        {
            ktxTexture* kTexture;
            KTX_error_code ktxresult;
            ktxresult = ktxTexture_CreateFromNamedFile(
                JseResourceManager::get_resource("textures/test/WoodenPlanks01_2048.ktx2").c_str(),
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

            auto& img = *R.CreateCommand<JseCmdCreateImage>();
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
            img.info.samplerDescription = R.FrameAlloc<JseSamplerDescription>();
            img.info.samplerDescription->minFilter = JseFilter::LINEAR_MIPMAP_LINEAR;
            img.info.samplerDescription->magFilter = JseFilter::NEAREST;
            img.info.samplerDescription->lodBias = 0.f;
            img.info.samplerDescription->maxAnisotropy = 1.0f;
            img.info.samplerDescription->maxLod = kt2->numLevels;
            img.info.samplerDescription->tilingS = JseImageTiling::REPEAT;
            img.info.samplerDescription->tilingT = JseImageTiling::REPEAT;
            img.info.samplerDescription->tilingR = JseImageTiling::REPEAT;

            R.Frame();

            ktxTexture_IterateLoadLevelFaces(kTexture, imageCB, &R);
        }

        auto& bind = *R.CreateCommand<JseCmdBindGraphicsPipeline>();
        bind.pipeline = JseGrapicsPipelineID{ 1 };

        auto& bb = *R.CreateCommand<JseCmdBindVertexBuffers>();
        bb.bindingCount = 1;
        bb.firstBinding = 0;
        bb.pBuffers = R.FrameAlloc<JseBufferID>(1);
        bb.pBuffers[0] = JseBufferID{ 1 };
        bb.pOffsets = R.FrameAlloc<JseDeviceSize>(1);
        bb.pOffsets[0] = 0;
        bb.pipeline = JseGrapicsPipelineID{ 1 };

        auto texSet = JseDescriptorSetID(R.NextID());
        auto& crdset = *R.CreateCommand<JseCmdCreateDescriptorSet>();
        crdset.info.setId = texSet;
        crdset.info.setLayoutId = texLayout;

        auto uniformSet = JseDescriptorSetID(R.NextID());
        auto& crdset2 = *R.CreateCommand<JseCmdCreateDescriptorSet>();
        crdset2.info.setId = uniformSet;
        crdset2.info.setLayoutId = uniformLayout;

        auto* wrdset = R.CreateCommand<JseCmdWriteDescriptorSet>();
        wrdset->info.descriptorCount = 1;
        wrdset->info.descriptorType = JseDescriptorType::SAMPLED_IMAGE;
        wrdset->info.dstBinding = 0;
        wrdset->info.setId = texSet;
        wrdset->info.pImageInfo = R.FrameAlloc<JseDescriptorImageInfo>();
        wrdset->info.pImageInfo[0].image = JseImageID{ 1 };

        auto& bindset = *R.CreateCommand<JseCmdBindDescriptorSets>();
        bindset.descriptorSetCount = 2;
        bindset.firstSet = 0;
        bindset.pDescriptorSets = R.FrameAlloc<JseDescriptorSetID>(2);
        bindset.pDescriptorSets[0] = texSet;
        bindset.pDescriptorSets[1] = uniformSet;

        R.Frame();


        ivec2 screen{};
        R.GetCore()->GetSurfaceDimension(screen);
        auto scissor_w = screen.x / 2;
        auto scissor_h = screen.y / 2;

        auto renderPass = JseCmdBeginRenderpass{};
        renderPass.info.colorClearEnable = true;
        renderPass.info.colorClearValue.color = glm::vec4{ 0.4f,0.0f,0.4f,1.0f };
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

        float tick = SCAST(float, SDL_GetTicks64()) / 1000.f;
        float angle = 0.f;
        float time1 = tick;

        vec3 viewOrigin{ 0.f,0.f,-2.f };

        mat4 W(1.0f), V, P;

        while(running) {

            R.SubmitCommand(renderPass);


            viewOrigin.z = 0.1f + (2.0f + 2.0f * std::sinf(radians(angle)));

            {
                auto& descWriter = *R.CreateCommand<JseCmdWriteDescriptorSet>();

                descWriter.info.descriptorCount = 1;
                descWriter.info.descriptorType = JseDescriptorType::INLINE_UNIFORM_BLOCK;
                descWriter.info.dstBinding = 100;
                descWriter.info.setId = uniformSet;

                W = mat4(1.0f);
                W = rotate(W, radians(angle), vec3(0.f, 1.f, 0.f));
                W = rotate(W, radians(00.0f), vec3(1.f, 0.f, 0.f));
                V = lookAt(viewOrigin, vec3{ 0.f,0.f,0.f }, vec3{ 0.f,1.f,0.f });
                P = perspective(radians(75.0f), (float)screen.x / screen.y, 0.01f, 100.f);
                descWriter.info.pUniformInfo = R.FrameAlloc<JseDescriptorUniformInfo>();
                descWriter.info.pUniformInfo[0].value = P * V * W;
                std::strcpy(descWriter.info.pUniformInfo[0].name, "g_W");

                auto* bindset = R.CreateCommand<JseCmdBindDescriptorSets>();
                bindset->descriptorSetCount = 1;
                bindset->firstSet = 0;
                bindset->pDescriptorSets = R.FrameAlloc<JseDescriptorSetID>(1);
                bindset->pDescriptorSets[0] = uniformSet;

                auto* draw = R.CreateCommand<JseCmdDraw>();
                draw->instanceCount = 1;
                draw->mode = JseTopology::Triangles;
                draw->vertexCount = 6;
            }
            {
                auto& descWriter = *R.CreateCommand<JseCmdWriteDescriptorSet>();

                descWriter.info.descriptorCount = 1;
                descWriter.info.descriptorType = JseDescriptorType::INLINE_UNIFORM_BLOCK;
                descWriter.info.dstBinding = 100;
                descWriter.info.setId = uniformSet;

                W = mat4(1.0f);
                W = rotate(W, radians(90.0f+angle), vec3(0.f, 1.f, 0.f));
                W = rotate(W, radians(00.0f), vec3(1.f, 0.f, 0.f));
                V = lookAt(viewOrigin, vec3{ 0.f,0.f,0.f }, vec3{ 0.f,1.f,0.f });
                P = perspective(radians(75.0f), (float)screen.x / screen.y, 0.01f, 100.f);
                descWriter.info.pUniformInfo = R.FrameAlloc<JseDescriptorUniformInfo>();
                descWriter.info.pUniformInfo[0].value = P * V * W;
                std::strcpy(descWriter.info.pUniformInfo[0].name, "g_W");

                auto* bindset = R.CreateCommand<JseCmdBindDescriptorSets>();
                bindset->descriptorSetCount = 1;
                bindset->firstSet = 0;
                bindset->pDescriptorSets = R.FrameAlloc<JseDescriptorSetID>(1);
                bindset->pDescriptorSets[0] = uniformSet;

                auto* draw = R.CreateCommand<JseCmdDraw>();
                draw->instanceCount = 1;
                draw->mode = JseTopology::Triangles;
                draw->vertexCount = 6;
            }


            R.Frame();

            I.ProcessEvents();

            float now = SCAST(float, SDL_GetTicks64()) / 1000.f;
            dt = now - tick;
            tick = now;

            angle += 30.f * dt;
            if (angle > 360.f) angle -= 360.f;
        }
    }
    catch (std::exception e) { Error("error=%s", e.what()); }
    
    JseShutdown();

	Info("Program terminated");

	return 0;
}