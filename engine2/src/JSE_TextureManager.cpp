#include "JSE.h"

#include <stb_image.h>
#include <nv_dds.h>
#include <ktx.h>
#include <vulkan/vulkan_core.h>
#include <ktxvulkan.h>
#include <gl_format.h>

namespace js {

    struct JseFormat_t {
        JseFormat fmt;
        bool srgb;
        //JseFormat_t() = default;
    };

	static JseFormat_t MapGLCompressedFmt(uint32_t f) {
        switch (f)
        {
        case 0x83F0: return { JseFormat::RGB_DXT1, true };
        case 0x83F1: return { JseFormat::RGBA_DXT1,true };
        case 0x83F2: return { JseFormat::RGBA_DXT3, true };
        case 0x83F3: return { JseFormat::RGBA_DXT5, true };
        case 0x8E8D:
        case 0x8E8E: return { JseFormat::RGBA_BPTC, true };
        default:
            return { JseFormat::RGBA8, true };
        }
    }


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
        auto* C = D->hwr->CreateCommand<JseCmdUploadImage>();

        auto* pixeldata = D->hwr->FrameAlloc<uint8_t>(faceLodSize);
        std::memcpy(pixeldata, pixels, faceLodSize);

        C->info.data = pixeldata;
        C->info.imageId = D->image;
        C->info.face = face;
        C->info.width = width;
        C->info.height = height;
        C->info.level = miplevel;
        C->info.depth = depth;
        C->info.imageSize = faceLodSize;

        Info("level %d, face %d, width %d, height %d, depth %d, size %d", miplevel, face, width, height, depth, faceLodSize);
        return KTX_SUCCESS;
    }


    Result TextureManager::load_ktx(JsString filename, JseImageID image, const JseSamplerDescription& sampler, Texture* resTex)
    {
        ktxTexture* kTexture;
        KTX_error_code ktxresult;
        bool tex_not_loaded = true;

        ktxresult = ktxTexture_CreateFromNamedFile(
            filename.c_str(),
            KTX_TEXTURE_CREATE_NO_FLAGS,
            &kTexture);

        if (ktxresult != KTX_SUCCESS) {
            return Result::GENERIC_ERROR;
        }

        JseImageTarget target{ kTexture->isArray ? JseImageTarget::D2_ARRAY : JseImageTarget::D2 };
        JseFormat_t format{};

        if (kTexture->classId == ktxTexture2_c)
        {
            ktxTexture2* kt2 = RCAST(ktxTexture2*, kTexture);
            if (ktxTexture2_NeedsTranscoding(kt2)) {
                ktx_texture_transcode_fmt_e tf = KTX_TTF_BC7_RGBA; // KTX_TTF_BC1_OR_3;
                ktxresult = ktxTexture2_TranscodeBasis(kt2, tf, 0);
                // Then use VkUpload or GLUpload to create a texture object on the GPU.
                if (ktxresult != KTX_SUCCESS) {
                    ktxTexture_Destroy(kTexture);
                    return Result::GENERIC_ERROR;
                }
                tex_not_loaded = false;
            }

            //auto fmt = s_vkf2jse_map.find(kt2->vkFormat);

        }

        auto fmt = s_vkf2jse_map.find(ktxTexture_GetVkFormat(kTexture));
        if (fmt == s_vkf2jse_map.end()) {
            ktxTexture_Destroy(kTexture);
            return Result::INVALID_VALUE;
        }

        format = fmt->second;

        if (kTexture->isCubemap) {
            target = kTexture->isArray ? JseImageTarget::CUBEMAP_ARRAY : JseImageTarget::CUBEMAP;
        }
        else if (kTexture->numDimensions == 1) {
            target = kTexture->isArray ? JseImageTarget::D1_ARRAY : JseImageTarget::D1;
        }
        else if (kTexture->numDimensions == 3) {
            target = kTexture->isArray ? JseImageTarget::D3_ARRAY : JseImageTarget::D3;
        }

        auto& img = *hwr_->CreateCommand<JseCmdCreateImage>();
        img.info.imageId = image;
        img.info.format = format.fmt;
        img.info.target = target;
        img.info.height = kTexture->baseHeight;
        img.info.width = kTexture->baseWidth;
        img.info.depth = kTexture->baseDepth;
        img.info.levelCount = kTexture->numLevels;
        img.info.srgb = format.srgb;
        img.info.compressed = kTexture->isCompressed;
        img.info.immutable = 1;
        img.info.samplerDescription = hwr_->FrameAlloc<JseSamplerDescription>();
        std::memcpy(img.info.samplerDescription, &sampler, sizeof(sampler));
        
        if (kTexture->numLevels > 1) { img.info.samplerDescription->maxLod = static_cast<float>(kTexture->numLevels - 1); }

        if (resTex)
        {
            resTex->format = format.fmt;
            resTex->imageId = image;
            resTex->width = kTexture->baseWidth;
            resTex->height = kTexture->baseHeight;
            resTex->layerCount = kTexture->numLayers;
            resTex->levelCount = kTexture->numLevels;
        }

        Ktx_imageCB_data_t data{ hwr_,image };

        if (tex_not_loaded) {
            ktxresult = ktxTexture_IterateLoadLevelFaces(kTexture, Ktx_image_callback, &data);
        }
        else {
            ktxresult = ktxTexture_IterateLevelFaces(kTexture, Ktx_image_callback, &data);
        }

        ktxTexture_Destroy(kTexture);

        if (ktxresult != KTX_SUCCESS) {
            Info("Cannot load texture [%s]", filename);
            return Result::GENERIC_ERROR;
        }

        return Result::SUCCESS;
    }

    TextureManager::TextureManager(GfxRenderer* gfxRenderer) : hwr_(gfxRenderer)
	{
	}
	Result TextureManager::LoadTexture(JsString resourceName, const JseSamplerDescription& sampler, Texture* resTex)
	{
        auto it = this->texture_map_.find(resourceName);
        if (it != std::end(this->texture_map_)) {
            return Result::ALREADY_EXISTS;
        }

        resourceName = Filesystem::get_resource(resourceName);
        if (resourceName.empty()) {
            return Result::NOT_EXISTS;
        }

        namespace fs = std::filesystem;
        JsString extension = fs::path(resourceName).extension().generic_string();
        JseImageID image = hwr_->CreateImage();

        resTex->imageId = image;

        if (extension == ".dds") { 
            // dds loader
        }
        else if (extension == ".ktx2" || extension == ".ktx") {
            // ktx loader
            return load_ktx(resourceName, image, sampler, resTex);
        }
        else {
            // stb_image loader
        }

        return Result::SUCCESS;
	}
}
