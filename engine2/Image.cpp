#include <filesystem>
#include <unordered_map>
#include <ktx.h>
#include <vulkan/vulkan_core.h>
#include <ktxvulkan.h>

#include <nv_dds.h>
#include <stb_image.h>
#include "./Image.h"
#include "./Logger.h"

namespace jsr {

	struct format_t {
		eImageFormat fmt;
		bool srgb;
	};

	static const std::unordered_map<ktx_uint32_t, format_t> s_vkf2jse_map
	{
	{VK_FORMAT_BC1_RGB_UNORM_BLOCK,         {IMF_RGB_COMPRESSED_DXT1,    false}},
	{VK_FORMAT_BC1_RGBA_UNORM_BLOCK,        {IMF_RGBA_COMPRESSED_DXT1,   false}},
	{VK_FORMAT_BC1_RGB_SRGB_BLOCK,          {IMF_RGB_COMPRESSED_DXT1,    true}},
	{VK_FORMAT_BC1_RGBA_SRGB_BLOCK,         {IMF_RGBA_COMPRESSED_DXT1,   true}},
	{VK_FORMAT_BC2_UNORM_BLOCK,             {IMF_RGBA_COMPRESSED_DXT3,   false}},
	{VK_FORMAT_BC2_SRGB_BLOCK,              {IMF_RGBA_COMPRESSED_DXT3,   true}},
	{VK_FORMAT_BC3_UNORM_BLOCK,             {IMF_RGBA_COMPRESSED_DXT5,   false}},
	{VK_FORMAT_BC3_SRGB_BLOCK,              {IMF_RGBA_COMPRESSED_DXT5,   true}},
	{VK_FORMAT_BC7_UNORM_BLOCK,             {IMF_RGBA_COMPRESSED_BC7,   false}},
	{VK_FORMAT_BC7_SRGB_BLOCK,              {IMF_RGBA_COMPRESSED_BC7,   true}}
	};

	static KTX_error_code Ktx_image_callback(int miplevel, int face, int width, int height, int depth, ktx_uint64_t faceLodSize, void* pixels, void* userdata)
	{

		Image* img = reinterpret_cast<Image*>(userdata);
#ifdef _DEBUG
		Info("level %d, face %d, width %d, height %d, depth %d, size %d", miplevel, face, width, height, depth, faceLodSize);
#endif // DEBUG
		img->UpdateImageData(width, height, miplevel, depth, face, (int)faceLodSize, pixels);

		return KTX_SUCCESS;
	}

	static bool imageLoader_KTX2(Image* img, const char* filename)
	{
		ktxTexture* kTexture;
		KTX_error_code ktxresult;
		bool tex_not_loaded = true;

		ktxresult = ktxTexture_CreateFromNamedFile(
			filename,
			KTX_TEXTURE_CREATE_NO_FLAGS,
			&kTexture);

		if (ktxresult != KTX_SUCCESS)
		{
			Error("ktxTexture_CreateFromNamedFile failed!");
			return false;
		}

		eImageShape target{ kTexture->isArray ? IMS_2D_ARRAY : IMS_2D };
		format_t format{};

		if (kTexture->classId != ktxTexture2_c)
		{
			Error("Not a KTX2 format image !");
			return false;
		}

		ktxTexture2* kt2 = (ktxTexture2*)kTexture;
		if (ktxTexture2_NeedsTranscoding(kt2))
		{
			ktx_texture_transcode_fmt_e tf = KTX_TTF_BC7_RGBA; // KTX_TTF_BC1_OR_3;
			ktxresult = ktxTexture2_TranscodeBasis(kt2, tf, 0);
			// Then use VkUpload or GLUpload to create a texture object on the GPU.
			if (ktxresult != KTX_SUCCESS) {
				ktxTexture_Destroy(kTexture);
				Error("KTX2 transcode failed");
				return false;
			}
			tex_not_loaded = false;
		}

		auto fmt = s_vkf2jse_map.find(ktxTexture_GetVkFormat(kTexture));
		if (fmt == s_vkf2jse_map.end())
		{
			Error("KTX2::format %d not found!", ktxTexture_GetVkFormat(kTexture));
			ktxTexture_Destroy(kTexture);
			return false;
		}

		format = fmt->second;
		if (kTexture->isCubemap)
		{
			target = kTexture->isArray ? IMS_CUBEMAP_ARRAY : IMS_CUBEMAP;
		}
		else if (kTexture->numDimensions == 1)
		{
			Error("1D texture not supported !");
			ktxTexture_Destroy(kTexture);
			return false;
		}
		else if (kTexture->numDimensions == 3)
		{
			Error("3D texture not supported !");
			ktxTexture_Destroy(kTexture);
			return false;
		}

		imageOpts_t opts{};
		opts.compressed = kTexture->isCompressed;
		opts.sizeX = kTexture->baseWidth;
		opts.sizeY = kTexture->baseHeight;
		opts.srgb = format.srgb;
		opts.format = format.fmt;
		opts.shape = target;
		opts.numLevel = kTexture->numLevels;
		opts.numLayer = kTexture->numLayers;
		opts.maxAnisotropy = 1.0f;

		img->opts = opts;
		img->Bind();

		if (tex_not_loaded)
		{
			ktxresult = ktxTexture_IterateLoadLevelFaces(kTexture, Ktx_image_callback, img);
		}
		else
		{
			ktxresult = ktxTexture_IterateLevelFaces(kTexture, Ktx_image_callback, img);
		}

		ktxTexture_Destroy(kTexture);

		if (ktxresult != KTX_SUCCESS) {
			Error("Cannot load texture [%s]", filename);
			return false;
		}

		if (kTexture->numLevels > 1)
		{
			img->SetFilter(IFL_LINEAR_LINEAR, IFL_LINEAR);
		}

		img->SetTextureParameters();

		return true;
	}

	static bool imageLoader_DDS(Image* img, const char* filename)
	{
		nv_dds::CDDSImage image;
		image.load(filename);

		if (!image.is_valid()) return false;

		img->Bind();
		switch (image.get_type())
		{
		case nv_dds::TextureFlat:
			image.upload_texture2D();
			break;
		case nv_dds::TextureCubemap:
			image.upload_textureCubemap();
			break;
		case nv_dds::Texture3D:
			image.upload_texture3D();
			break;
		}
	}

	static bool imageLoader_STB(Image* img, const char* filename)
	{
		if (img->IsCreated()) return false;
		
		if (stbi_is_hdr(filename))
		{
			Error("HDR image not supported yet!");
			return false;
		}

		int iw = 0, ih = 0, n = 0;
		int nr = 4;
		if (stbi_info(filename, &iw, &ih, &n))
		{
			nr = n == 3 ? 4 : n;
		}
		else
		{
			return false;
		}

		eImageFormat fmt = IMF_RGBA;
		stbi_set_flip_vertically_on_load(true);
		stbi_uc* data = stbi_load(filename, &iw, &ih, &n, nr);
		if (data == nullptr)
		{
			Error("stbi_load failed");
			return false;
		}

		switch (n)
		{
		case 1:	fmt = IMF_A8; break;
		case 2: fmt = IMF_RG8; break;
		case 4: fmt = IMF_RGBA; break;
		}
		
		imageOpts_t opts{};
		opts.compressed = false;
		opts.sizeX = iw;
		opts.sizeY = ih;
		opts.srgb = true;
		opts.format = fmt;
		opts.shape = IMS_2D;
		opts.numLevel = 1;
		opts.numLayer = 0;
		opts.maxAnisotropy = 1.0f;

		img->opts = opts;

		img->Bind();

		if (!img->UpdateImageData(iw, ih, 0, 0, 0, 0, data))
		{
			Error("Image::UpdateImageData failed");
			stbi_image_free(data);
			return false;
		}

		stbi_image_free(data);
		img->SetTextureParameters();

		return true;
	}



	Image::Image() : Image("_unnamed_") {}

	void Image::SetUsage(eImageUsage x)
	{
		opts.usage = x;
	}
	void Image::SetFilter(eImageFilter min, eImageFilter mag)
	{
		minFilter = min;
		magFilter = mag;
	}

	void Image::SetRepeat(eImageRepeat rS, eImageRepeat rT)
	{
		wrapS = rS;
		wrapT = rT;
	}

	void Image::SetNumLevel(int x)
	{
		opts.numLevel = x;
	}

	void Image::SetNumLayer(int x)
	{
		opts.numLayer = x;
	}

	void Image::SetMaxAnisotropy(float f)
	{
		opts.maxAnisotropy = f;
	}

	void Image::AddRef()
	{
		++refCount;
	}

	void Image::ReleaseRef()
	{
		if (refCount > 0) --refCount;
	}

	int Image::GetNumLevel() const
	{
		return opts.numLevel;
	}

	int Image::GetNumLayer() const
	{
		return opts.numLayer;
	}

	int Image::GetWidth() const
	{
		return opts.sizeX;
	}

	int Image::GetHeight() const
	{
		return opts.sizeY;
	}

	float Image::GetMaxAnisotropy() const
	{
		return opts.maxAnisotropy;
	}

	eImageFormat Image::GetFormat() const
	{
		return opts.format;
	}

	eImageShape Image::GetShape() const
	{
		return opts.shape;
	}

	eImageFilter Image::GetMinFilter() const
	{
		return minFilter;
	}

	eImageFilter Image::GetMagFilter() const
	{
		return magFilter;
	}

	eImageRepeat Image::GetRepeatS() const
	{
		return wrapS;
	}

	eImageRepeat Image::GetRepeatT() const
	{
		return wrapT;
	}

	eImageUsage Image::GetUsage() const
	{
		return opts.usage;
	}

	bool Image::IsCreated() const
	{
		return created;
	}

	bool Image::IsCompressed() const
	{
		return opts.compressed;
	}

	bool Image::Load(const char* filename)
	{
		std::filesystem::path path(filename);
		std::filesystem::path ext = path.extension();
		
		if (ext.string() == ".ktx2")
		{
			return imageLoader_KTX2(this, filename);
		}
		else if (ext.string() == ".dds")
		{
			// dds path
			return imageLoader_DDS(this, filename);
		}
		else
		{
			// stb_image path
			return imageLoader_STB(this, filename);
		}

		return false;
	}

	Image::~Image()
	{
		PurgeImage();
	}
}