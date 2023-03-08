#include "JSE.h"

#include <stb_image.h>
#include <nv_dds.h>
#include <ktx.h>
#include <gl_format.h>

namespace js {

	static JseFormat MapGLCompressedFmt(uint32_t f) {
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

    struct JseFormat_t {
        JseFormat fmt;
        bool srgb;
        //JseFormat_t() = default;
    };

    static const JsHashMap<ktx_uint32_t, JseFormat_t> s_vkf2jse_map{
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

    struct Ktx_imageCB_data_t {
        GfxRenderer* hwr;
        JseImageID image;
    };

    static KTX_error_code Ktx_image_callback(int miplevel, int face, int width, int height, int depth, ktx_uint64_t faceLodSize, void* pixels, void* userdata)
    {
        auto* D = static_cast<Ktx_imageCB_data_t*>(userdata);
        JseCmdUploadImage c{};

        //c.info.data = R->FrameAlloc<uint8_t>(faceLodSize);
        //std::memcpy(c.info.data, pixels, faceLodSize);

        c.info.data = (uint8_t*)pixels;
        c.info.imageId = D->image;
        c.info.face = face;
        c.info.width = width;
        c.info.height = height;
        c.info.level = miplevel;
        c.info.depth = 1;
        c.info.imageSize = faceLodSize;

        Info("level %d, face %d, width %d, height %d, depth %d, size %d", miplevel, face, width, height, depth, faceLodSize);
        if (D->hwr->UploadImage(c.info) == Result::SUCCESS)
            return KTX_SUCCESS;
        else
            return KTX_GL_ERROR;
    }


	TextureManager::TextureManager(GfxRenderer* gfxRenderer) : hwr_(gfxRenderer)
	{
	}
	Result TextureManager::LoadTexture(JsString fileName, Texture** tex)
	{
		return Result();
	}
}
