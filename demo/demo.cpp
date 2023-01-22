#include "JSE.h"
#include "JSE_GfxCoreGL46.h"

#define SCREEN_WIDTH 1440
#define SCREEN_HEIGHT 900
#define FULLSCREEN false


void demo_1();

int main(int argc, char** argv)
{
    
    JseInit(argc, argv);

    JseGfxRenderer R;

    try {
        JseBufferCreateInfo& bci = *R.FrameAlloc<JseBufferCreateInfo>();
        bci.bufferId = JseBufferID{ 1 };
        bci.size = 4 * 1024 * 1024;
        bci.target = JseBufferTarget::UNIFORM_DYNAMIC;
        bci.storageFlags = JSE_BUFFER_STORAGE_DYNAMIC_BIT;

        JseBufferUpdateInfo& bui = *R.FrameAlloc<JseBufferUpdateInfo>();
        bui.bufferId = JseBufferID{ 1 };
        bui.offset = 1024;
        bui.data = R.R_FrameAlloc(2 * 1024 * 1024);

        auto* c1 = R.GetCommandBuffer<JseGfxCmdCreateBuffer>();
        c1->info = bci;        

        JseImageCreateInfo& ici = *R.FrameAlloc<JseImageCreateInfo>();
        JseSamplerDescription& samplerDescription = *R.FrameAlloc<JseSamplerDescription>();
        samplerDescription.borderColor = JseColor4f{ 0.0f,0.0f,0.0f,1.0f };
        samplerDescription.lodBias = 0.0f;
        samplerDescription.minLod = 0.0f;
        samplerDescription.maxLod = 1000.0f;
        samplerDescription.maxAnisotropy = 1.0f;
        samplerDescription.magFilter = JseFilter::LINEAR;
        samplerDescription.minFilter = JseFilter::LINEAR;
        samplerDescription.tilingS = JseImageTiling::CLAMP_TO_EDGE;
        samplerDescription.tilingT = JseImageTiling::CLAMP_TO_EDGE;

        ici.imageId = JseImageID{ 1 };
        ici.target = JseImageTarget::D2_ARRAY;
        ici.width = 1024 * 2;
        ici.height = 1024 * 2;
        ici.depth = 16; // 16 layer
        ici.levelCount = static_cast<uint32_t>(std::log2(std::max(ici.width, ici.height))) + 1;
        ici.format = JseFormat::RGBA8;
        ici.sampler = JseSamplerID{};
        ici.samplerDescription = &samplerDescription;
    }
    catch (std::exception e) { Error("error=%s", e.what()); }

    R.Frame();

    JseShutdown();

	Info("Program terminated");

	return 0;
}