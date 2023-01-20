#include "engine/engine.h"
#include <SDL.h>
#include <iostream>
#include <filesystem>
#include <memory>
#include <tiny_gltf.h>
#include <cmath>
#include <type_traits>
#include <tuple>
#include "demo.h"
#include "logger.h"

#include "effect_test_engine.h"
#include "effect_bump.h"
#include "gpu.h"
#include "gpu_types.h"
#include "gpu_utils.h"
#include "window.h"

#include "engine/gfx/image.h"
#include "engine/gfx/material.h"
#include "engine/gfx/draw_vert.h"
#include "engine/resource/allocator.h"
#include "JSE.h"

#define SCREEN_WIDTH 1440
#define SCREEN_HEIGHT 900
#define FULLSCREEN false


void App_EventLoop()
{
    SDL_Event e;
    bool running = true;
    float prev = float(SDL_GetTicks());

    std::unique_ptr<Effect> activeEffect = std::make_unique<BumpEffect>();

    if (!activeEffect->Init())
        return;

    int sampleCount = 0;
    Uint32 ticks = 0;
    uint64_t frame_num{ 0 };

    gfx::Renderer* hwr = ctx::Context::default()->hwr();

    while (running)
    {
        float now = float(SDL_GetTicks());
        float time = now - prev;
        prev = now;

        running = activeEffect->Update(time);

        while (SDL_PollEvent(&e) != SDL_FALSE && running)
        {
            running = activeEffect->HandleEvent(&e, time);

            if (e.type == SDL_QUIT)
            {
                running = false;
            }
            else if(e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_ESCAPE)
                {
                    running = false;
                }
            }
        }


        Uint32 tick1 = SDL_GetTicks();
        if (!activeEffect->Render(frame_num))
        {
            running = false;
        }

        hwr->frame();

        Uint32 tick2 = SDL_GetTicks();
        ticks += tick2 - tick1;
        if (++sampleCount > 2000)
        {
            SDL_Log("ftime: %f", (ticks / 2000.0f));
            sampleCount = 0;
            ticks = 0;
        }
        ++frame_num;
    }
}

namespace fs = std::filesystem;

void traverse_node(tinygltf::Model const& model, int node, int level = 0)
{
    std::string s{};
    for (int i = 0; i < level; ++i) s += "\t";

    tinygltf::Node const& N = model.nodes[node];

    {
        Info("--------------------------------------------------------------------------");
        Info("%s Node %s", s.c_str(), N.name.c_str());
        if (N.matrix.size())
        {
            Info("%s matrix", s.c_str());
        }
        if (N.rotation.size())
        {
            Info("%s rotation (%f,%f,%f,%f)", s.c_str(), N.rotation[0], N.rotation[1], N.rotation[2], N.rotation[3]);
        }
        if (N.scale.size())
        {
            Info("%s scale (%f,%f,%f)", s.c_str(), N.scale[0], N.scale[1], N.scale[2]);
        }
        if (N.translation.size())
        {
            Info("%s translation (%f,%f,%f)", s.c_str(), N.translation[0], N.translation[1], N.translation[2]);
        }
    }

    if (N.mesh >= 0)
    {
        tinygltf::Mesh const& M = model.meshes[N.mesh];
        Info("%s Mesh %d %s pr:%d", s.c_str(), N.mesh, M.name.c_str(),M.primitives.size());
    }
    else if (N.camera >= 0)
    {
        tinygltf::Camera const& C = model.cameras[N.camera];
        Info("%s Camera %d %s fov:%f", s.c_str(), N.camera, C.name.c_str(), glm::degrees( C.perspective.yfov ));
    }
    else if (N.extensions.size())
    {
        for (auto const& ext : N.extensions)
        {
            Info("%s Ext: %s", s.c_str(), ext.first.c_str());
            if (ext.first == "KHR_lights_punctual")
            {
                int light = ext.second.Get("light").GetNumberAsInt();
                auto const& L = model.lights[light];            
                Info("%s Light %d, Typ:%s", s.c_str(), light, L.type.c_str());
            }
        }
    }

    for (int child : model.nodes[node].children)
    {
        traverse_node(model, child, level + 1);
    }
}
 
#include "JSE_GfxCoreGL46.h"

int SDL_main(int argc, char** argv)
{
    JseInit(argc, argv);
    JseGfxCore* core = new JseGfxCoreGL();
    core->Init(true);

    JseSurfaceCreateInfo sci{};
    sci.width = 1440;
    sci.height = 900;
    sci.colorBits = 24;
    sci.depthBits = 24;
    sci.stencilBits = 8;
    sci.alphaBits = 8;
    sci.swapInterval = 1;
    if (core->CreateSurface(sci) == JseResult::SUCCESS) {
        Info("Surface created !");
    }
    else {
        exit(1);
    }

    core->SetVSyncInterval(-1);

    JseDeviceCapabilities cap{};
    core->GetDeviceCapabilities(cap);
    Info("Renderer: %s", cap.pRenderer);
    Info("Renderer ver: %s", cap.pRendererVersion);

    JseBufferCreateInfo bci{};
    bci.bufferId = JseBufferID{ 1 };
    bci.size = 4 * 1024 * 1024;
    bci.target = JseBufferTarget::UNIFORM_DYNAMIC;
    bci.storageFlags = JSE_BUFFER_STORAGE_DYNAMIC_BIT;
    if (core->CreateBuffer(bci) == JseResult::SUCCESS) {
        Info("Buffer Created");
    }

    JseMemory mem{ 1024 * 1024 * 2 };
    JseBufferUpdateInfo bui{};
    bui.bufferId = JseBufferID{ 1 };
    bui.offset = 1024;
    bui.data = mem;
    if (core->UpdateBuffer(bui) == JseResult::SUCCESS) {
        Info("Buffer updated");
    }

    JseImageCreateInfo ici{};
    JseSamplerDescription samplerDescription{};
    samplerDescription.borderColor = JseColor4f{ 0.0f,0.0f,0.0f,1.0f };
    samplerDescription.lodBias = 0.0f;
    samplerDescription.minLod = 0.0f;
    samplerDescription.maxLod = 1000.0f;
    samplerDescription.maxAnisotropy = 1.0f;
    samplerDescription.magFilter = JseFilter::LINEAR;
    samplerDescription.minFilter = JseFilter::LINEAR;
    samplerDescription.tilingS = JseImageTiling::CLAMP_TO_EDGE;
    samplerDescription.tilingT = JseImageTiling::CLAMP_TO_EDGE;

    JseMemory txData(128 * 128 * 4);
    ici.imageId = JseImageID{ 1 };
    ici.target = JseImageTarget::D2_ARRAY;
    ici.width = 1024*2;
    ici.height = 1024*2;
    ici.depth = 16; // 16 layer
    ici.levelCount = static_cast<uint32_t>(std::log2(std::max(ici.width, ici.height))) + 1;
    ici.format = JseFormat::RGBA8;
    ici.sampler = JseSamplerID{};
    ici.samplerDescription = &samplerDescription;

    if (core->CreateTexture(ici) == JseResult::SUCCESS) {
        Info("Texture created");
    }

    ici.imageId = JseImageID{ 2 };
    ici.target = JseImageTarget::D2;
    ici.width = 1024;
    ici.height = 1024;
    ici.depth = 1; // 16 layer
    ici.levelCount = static_cast<uint32_t>(std::log2(std::max(ici.width, ici.height))) + 1;
    ici.format = JseFormat::RG11B10F;
    ici.sampler = JseSamplerID{};
    ici.samplerDescription = &samplerDescription;

    if (core->CreateTexture(ici) == JseResult::SUCCESS) {
        Info("Texture created");
    }

    ici.imageId = JseImageID{ 3 };
    ici.target = JseImageTarget::D2;
    ici.width = 1024;
    ici.height = 1024;
    ici.depth = 1; // 16 layer
    ici.levelCount = static_cast<uint32_t>(std::log2(std::max(ici.width, ici.height))) + 1;
    ici.format = JseFormat::D32F;
    ici.sampler = JseSamplerID{};
    ici.samplerDescription = &samplerDescription;

    if (core->CreateTexture(ici) == JseResult::SUCCESS) {
        Info("Texture created");
    }

    JseFrameBufferAttachmentDescription color{};
    JseFrameBufferAttachmentDescription depth{};
    color.image = JseImageID{ 2 };
    depth.image = JseImageID{ 3 };
    
    JseFrameBufferCreateInfo fbci{};
    fbci.frameBufferId = JseFrameBufferID{ 1 };
    fbci.colorAttachmentCount = 1;
    fbci.pColorAttachments = &color;
    fbci.pDepthAttachment = &depth;
    fbci.width = 1024;
    fbci.height = 1024;
    if (core->CreateFrameBuffer(fbci) == JseResult::SUCCESS) {
        Info("Framebuffer created");
    }

    JseImageUploadInfo iui{};
    iui.imageId = JseImageID{ 1 };
    iui.level = 0;
    iui.width = 128;
    iui.height = 128;
    iui.depth = 1;
    iui.xoffset = 0;
    iui.yoffset = 0;
    iui.zoffset = 2; // layer
    iui.face = JseCubeMapFace::POSITIVE_Y;
    iui.data = txData;
    if (core->UpdateImageData(iui) == JseResult::SUCCESS) {
        Info("Texture updated");
    }

    if (core->DestroyBuffer(JseBufferID{ 1 }) == JseResult::SUCCESS) {
        Info("Buffer deleted");
    }

    if (core->UpdateBuffer(bui) != JseResult::SUCCESS) {
        Info("Buffer update failed");
    }

    JseShaderCreateInfo shci_vert{};
    shci_vert.shaderId = JseShaderID{ 1 };
    shci_vert.stage = JseShaderStage::VERTEX;
    shci_vert.codeSize = 0;
    shci_vert.pCode = R"(
#version 450

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec4 in_Color;

struct cb_Input {
    vec4 a;
    vec4 b;
};

uniform cb_Input vs_input;

out vec4 vofi_Color;

void main() {
    gl_Position = in_Position * vs_input.b;
    vofi_Color = in_Color * vs_input.a;
}
)";
    std::string err;
    if (core->CreateShader(shci_vert, err) != JseResult::SUCCESS) {
        Info("Shader compiler error: %s", err.c_str());
        goto _exit;
    }

    JseShaderCreateInfo shci_frag{};
    shci_frag.shaderId = JseShaderID{ 2 };
    shci_frag.stage = JseShaderStage::FRAGMENT;
    shci_frag.codeSize = 0;
    shci_frag.pCode = R"(
#version 450

layout(location = 0) out vec4 fragColor;
in vec4 vofi_Color;

void main() {
    fragColor = vofi_Color;
}
)";
    if (core->CreateShader(shci_frag, err) != JseResult::SUCCESS) {
        Info("Shader compiler error: %s", err.c_str());
        goto _exit;
    }

    JseGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    JsePipelineShaderStageCreateInfo pipelineShaderStageCreateInfo[2]{};
    
    JsePipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
    JseVertexInputBindingDescription vertexInputBindingDescription{};
    JseVertexInputAttributeDescription vertexInputAttributeDescription[2]{};
    
    pipelineShaderStageCreateInfo[0].stage = JseShaderStage::VERTEX;
    pipelineShaderStageCreateInfo[0].shader = JseShaderID{ 1 };
    pipelineShaderStageCreateInfo[1].stage = JseShaderStage::FRAGMENT;
    pipelineShaderStageCreateInfo[1].shader = JseShaderID{ 2 };

    vertexInputBindingDescription.binding = 0;
    vertexInputBindingDescription.inputRate = JseVertexInputRate::VERTEX;
    vertexInputBindingDescription.stride = 20;
    vertexInputAttributeDescription[0].bindig = 0;
    vertexInputAttributeDescription[0].format = JseFormat::RGBA32F;
    vertexInputAttributeDescription[0].location = 0;
    vertexInputAttributeDescription[0].offset = 0;
    vertexInputAttributeDescription[1].bindig = 0;
    vertexInputAttributeDescription[1].format = JseFormat::RGBA8;
    vertexInputAttributeDescription[1].location = 1;
    vertexInputAttributeDescription[1].offset = 16;

    pipelineVertexInputStateCreateInfo.attributeCount = 2;
    pipelineVertexInputStateCreateInfo.bindingCount = 1;
    pipelineVertexInputStateCreateInfo.pBindings = &vertexInputBindingDescription;
    pipelineVertexInputStateCreateInfo.pAttributes = vertexInputAttributeDescription;
    
    graphicsPipelineCreateInfo.graphicsPipelineId = JseGrapicsPipelineID{ 1 };
    graphicsPipelineCreateInfo.stageCount = 2;
    graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
    graphicsPipelineCreateInfo.stageCount = 2;
    graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfo;
    graphicsPipelineCreateInfo.renderState = GLS_DEPTHFUNC_LESS | GLS_CULL_BACKSIDED;
    graphicsPipelineCreateInfo.setLayoutId = JseDescriptorSetLayoutID{ 1 };

    JseDescriptorSetLayoutCreateInfo setCreate{};
    JseDescriptorSetLayoutBinding layout_bindings[]{
        {
            2,
            JseDescriptorType::SAMPLED_IMAGE,
            1,
            JSE_STAGE_FLAG_FRAGMENT
        },
        {
            3,
            JseDescriptorType::UNIFORM_BUFFER,
            1,
            JSE_STAGE_FLAG_ALL
        }
    };

    setCreate.bindingCount = 2;
    setCreate.pBindings = layout_bindings;
    setCreate.setLayoutId = JseDescriptorSetLayoutID{ 1 };

    if (core->CreateDescriptorSetLayout(setCreate) == JseResult::SUCCESS) {
        Info("Layout creted");
    }

    if (core->CreateGraphicsPipeline(graphicsPipelineCreateInfo) == JseResult::SUCCESS) {
        Info("Graphics pipeline created");
    }

    JseRenderPassInfo pass{};
    pass.framebuffer = JseFrameBufferID{ 1 };
    pass.colorClearEnable = true;
    pass.depthClearEnable = true;
    pass.depthClearValue.depth = 1.0f;
    pass.colorClearValue.color = JseColor4f{ 0.f,0.f,0.f,1.f };
    pass.viewport = JseRect2D{ 0,0,1024,1024 };
    core->BeginRenderPass(pass);

    core->BindGraphicsPipeline(JseGrapicsPipelineID{ 1 });
    core->DeleteGraphicsPipeline(JseGrapicsPipelineID{ 1 });

    core->EndRenderPass();


    _exit:
    core->Shutdown();
    JseShutdown();

    exit(0);

    rc::FileSystem::set_working_dir(fs::absolute(fs::path("../")).generic_string());

    rc::ResourceManager::add_resource_path("../assets/shaders");
    rc::ResourceManager::add_resource_path("../assets/textures");
    rc::ResourceManager::add_resource_path("../assets/models");

    using namespace tinygltf;
    using namespace std;

    TinyGLTF loader;
    Model model;
    string warn;

    Info("sizeof(Model)=%d", sizeof(Model));

    auto file = rc::ResourceManager::get_resource("models/scene.gltf");
    if (loader.LoadASCIIFromFile(&model, &err, &warn, file))
    {
        Info("Version: %s, %s", model.asset.version.c_str(), model.asset.generator.c_str());
        for (auto& e : model.extensionsRequired) {
            Info("%s", e.c_str());
        }
    }

    for (int node : model.scenes[model.defaultScene].nodes)
    {
        traverse_node(model, node, 0);
    }

    Info("V_Init Start");

    gfx::Renderer* hwr = ctx::Context::default()->hwr();

    if (!hwr->init(gfx::RendererType::OpenGL, 1440, 900, "test", 1))
    {
        return false;
    }

    App_EventLoop();

	Info("Program terminated");

	return 0;
}