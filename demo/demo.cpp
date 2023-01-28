#include "JSE.h"
#include "JSE_GfxCoreGL46.h"
#include <nv_dds.h>

#define SCREEN_WIDTH 1440
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
    default:
        return JseFormat::RGBA8;
    }
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

void main() {
    gl_Position = vec4(in_Position, 1.0);
    vofi_TexCoord = in_TexCoord;
}
)" };

    const char* fragshader{ R"(
#version 450

in vec2 vofi_TexCoord;

out vec4 fragColor;

void main() {
    fragColor = vec4(vofi_TexCoord.x, vofi_TexCoord.y, 0.0, 1.0);
}
)" };

    try {
        JseGfxRenderer R;
        JseInputManager I;

        I.SetOnExitEvent([&]
            {
                running = false;
            });

        I.SetOnKeyboardEvent([&](JseKeyboardEvent e)
            {
                Info("Key event: %x:%04x", e.type, e.keysym.sym);
            });

        R.SetCore(new JseGfxCoreGL());
        R.InitCore(1024, 768, false, false);

        JseCreateShaderCommand& sp1 = *R.GetCommandBuffer<JseCreateShaderCommand>();
        sp1.info.pCode = vtxshader;
        sp1.info.shaderId = JseShaderID{ 1 };
        sp1.info.stage = JseShaderStage::VERTEX;
        JseCreateShaderCommand& sp2 = *R.GetCommandBuffer<JseCreateShaderCommand>();
        sp2.info.pCode = fragshader;
        sp2.info.shaderId = JseShaderID{ 2 };
        sp2.info.stage = JseShaderStage::FRAGMENT;

        JseCreateDescriptorSetLayoutBinding& setBinding = *R.GetCommandBuffer<JseCreateDescriptorSetLayoutBinding>();
        setBinding.info.setLayoutId = JseDescriptorSetLayoutID{ 1 };
        setBinding.info.bindingCount = 0;

        JseCreateGraphicsPipelineCommand& p = *R.GetCommandBuffer<JseCreateGraphicsPipelineCommand>();
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

        JseCreateBufferCommand& buf = *R.GetCommandBuffer<JseCreateBufferCommand>();
        buf.info.bufferId = JseBufferID{ 1 };
        buf.info.size = 64 * 1024;
        buf.info.storageFlags = JSE_BUFFER_STORAGE_DYNAMIC_BIT;
        buf.info.target = JseBufferTarget::VERTEX;

        JseUpdateBufferCommand& upd = *R.GetCommandBuffer<JseUpdateBufferCommand>();
        upd.info.bufferId = JseBufferID{ 1 };
        upd.info.size = sizeof(rect);
        upd.info.data = (uint8_t*)&rect[0];

        JseBindGraphicsPipelineCommand& bind = *R.GetCommandBuffer<JseBindGraphicsPipelineCommand>();
        bind.pipeline = JseGrapicsPipelineID{ 1 };

        JseBindVertexBuffersCommand& bb = *R.GetCommandBuffer<JseBindVertexBuffersCommand>();
        bb.bindingCount = 1;
        bb.firstBinding = 0;
        bb.pBuffers = R.FrameAlloc<JseBufferID>(1);
        bb.pBuffers[0] = JseBufferID{ 1 };
        bb.pOffsets = R.FrameAlloc<JseDeviceSize>(1);
        bb.pOffsets[0] = 0;

        {
            CDDSImage image;
            image.load("d:/tokio.dds");

            JseImageTarget target{JseImageTarget::D2};

            if (image.is_cubemap()) {
                target = JseImageTarget::CUBEMAP;
            }
            else if (image.get_height() == 0) {
                target = JseImageTarget::D1;
            }
            else if (image.is_volume()) {
                target = JseImageTarget::D3;
            }

            JseCreateImageCommand& img = *R.GetCommandBuffer<JseCreateImageCommand>();
            img.info.imageId = JseImageID{ 1 };
            img.info.format = MapGLCompressedFmt(image.get_format());
            img.info.target = target;
            img.info.height = image.get_height();
            img.info.width = image.get_width();
            img.info.levelCount = 1+image.get_num_mipmaps();
            //img.info.initAfterCreate = true;

            for (int face = 0; face < 6; ++face) {
                auto& cface = image.get_cubemap_face(face);

                JseCubeMapFace f = JSE_CUBE_MAP_FACES[face];

                JseUploadImageCommand& upl = *R.GetCommandBuffer<JseUploadImageCommand>();
                upl.info.imageId = JseImageID{ 1 };
                upl.info.face = f;
                upl.info.width = cface.get_width();
                upl.info.height = cface.get_height();
                upl.info.level = 0;
                upl.info.data = cface;
                upl.info.imageSize = cface.get_size();

                for (int k = 0; k < cface.get_num_mipmaps(); ++k) {
                    auto& level = cface.get_mipmap(k);
                    JseUploadImageCommand& upl = *R.GetCommandBuffer<JseUploadImageCommand>();
                    upl.info.imageId = JseImageID{ 1 };
                    upl.info.face = f;
                    upl.info.width = level.get_width();
                    upl.info.height = level.get_height();
                    upl.info.level = k+1;
                    upl.info.data = level;
                    upl.info.imageSize = level.get_size();
                }
            }
            R.Frame();
        }


        while(running) {
            JseBeginRenderpassCommand& x = *R.GetCommandBuffer<JseBeginRenderpassCommand>();
            x.info.colorClearEnable = true;
            x.info.colorClearValue.color = JseColor4f{ 0.4f,0.0f,0.4f,1.0f };
            x.info.scissorEnable = false;
            x.info.framebuffer = JseFrameBufferID{ 0 };
            x.info.viewport.x = 0;
            x.info.viewport.y = 0;
            x.info.viewport.w = 1024;
            x.info.viewport.h = 768;
            x.info.scissor.w = 512;
            x.info.scissor.h = 512;

            JseDrawCommand& draw = *R.GetCommandBuffer<JseDrawCommand>();
            draw.firstInstance = 0;
            draw.firstVertex = 0;
            draw.instanceCount = 1;
            draw.mode = JseTopology::Triangles;
            draw.vertexCount = 6;
            
            R.Frame();
            I.ProcessEvents();
        }
    }
    catch (std::exception e) { Error("error=%s", e.what()); }

    JseShutdown();

	Info("Program terminated");

	return 0;
}