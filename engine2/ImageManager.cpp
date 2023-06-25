#include <algorithm>
#include "./Logger.h"
#include "./ImageManager.h"
#include "./Image.h"
#include "./RenderSystem.h"
#include <random>

namespace jsr {

	globalImages_t globalImages;

	ImageManager::ImageManager() : initialized(false) {}

	ImageManager::~ImageManager()
	{
		Shutdown();
	}
	bool ImageManager::Init()
	{

		if (renderSystem.backend->IsInitialized() == false)
		{
			Error("[ImageManager]: render system not initialized !");
			return false;
		}

		globalImages.GBufferFragPos = AllocImage("_gbuffer_fragpos");
		globalImages.GBufferAlbedo = AllocImage("_gbuffer_albedo");
		globalImages.GBufferNormal = AllocImage("_gbuffer_normal");
		globalImages.GBufferDepth = AllocImage("_gbuffer_depth");
		globalImages.GBufferSpec = AllocImage("_gbuffer_specular");
		globalImages.Shadow = AllocImage("_shadow");
		globalImages.defaultImage = AllocImage("_defaultImage");
		globalImages.HDRaccum = AllocImage("_hdrimage");
//		globalImages.HDRdepth = AllocImage("_hdrDepth");
		globalImages.whiteImage = AllocImage("_white");
		globalImages.grayImage = AllocImage("_gray");
		globalImages.blackImage = AllocImage("_black");
		globalImages.flatNormal = AllocImage("_flatnormal");
		globalImages.ssaoNoise = AllocImage("_ssaoNoise");
		globalImages.ssaoMap = AllocImage("_ssaoMap");
		globalImages.ssaoblur[0] = AllocImage("_ssaoBlur0");
		globalImages.ssaoblur[1] = AllocImage("_ssaoBlur1");

		for (int i = 0; i < 2; ++i)
		{
			globalImages.HDRblur[i] = AllocImage("_hdrblur_" + std::to_string(i));
		}

		globalImages.HDRbloom.resize(renderGlobals.bloomDownsampleLevel);
		for (int i = 0; i < 2; ++i)
		{
			globalImages.HDRbloom[i] = AllocImage("_hdrbloom_" + std::to_string(i));
		}

		int screen_width, screen_height;
		renderSystem.backend->GetScreenSize(screen_width, screen_height);

		imageOpts_t opts{};
		opts.format = IMF_RGBA;
		opts.shape = IMS_2D;
		opts.usage = IMU_DIFFUSE;
		opts.sizeX = screen_width;
		opts.sizeY = screen_height;
		opts.numLevel = 1;
		opts.numLayer = 1;
		opts.maxAnisotropy = 1.0f;
		opts.autocompress = false;
		opts.srgb = true;

		if (!globalImages.GBufferAlbedo->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
		{
			Error("[ImageManager]: Image GBufferAlbedo allocation failed !");
		}

		opts.format = IMF_R32F;
		opts.usage = IMU_FRAGPOS;
		opts.srgb = false;
		if (!globalImages.GBufferFragPos->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
		{
			Error("[ImageManager]: Image GBufferFragPos allocation failed !");
		}

		opts.format = IMF_RG16F;
		opts.usage = IMU_NORMAL;
		if (!globalImages.GBufferNormal->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
		{
			Error("[ImageManager]: Image GBufferNormal allocation failed !");
		}

		opts.format = IMF_RGBA;
		opts.usage = IMU_AORM;
		if (!globalImages.GBufferSpec->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
		{
			Error("[ImageManager]: Image GBufferSpec allocation failed !");
		}

		opts.format = IMF_R11G11B10F;
		opts.usage = IMU_HDR;
		if (!globalImages.HDRaccum->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
		{
			Error("[ImageManager]: Image HDRaccum allocation failed !");
		}

		const int bloomDiv = 1 << renderGlobals.bloomDownsampleLevel;
		opts.sizeX = screen_width / bloomDiv;
		opts.sizeY = screen_height / bloomDiv;
		for (int i = 0; i < 2; ++i)
		{
			if (!globalImages.HDRblur[i]->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
			{
				Error("[ImageManager]: Image HDRblur allocation failed !");
			}
		}

		opts.sizeX = screen_width ;
		opts.sizeY = screen_height ;
		for (int i = 0; i < renderGlobals.bloomDownsampleLevel; ++i)
		{
			opts.sizeX /= 2;
			opts.sizeY /= 2;

			if (!globalImages.HDRbloom[i]->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
			{
				Error("[ImageManager]: Image HDRbloom allocation failed !");
			}
		}

		opts.sizeX = screen_width / 2;
		opts.sizeY = screen_height / 2;
		opts.format = IMF_R8;
		opts.usage = IMU_AORM;
		if (!globalImages.ssaoMap->AllocImage(opts, IFL_NEAREST, IMR_CLAMP_TO_EDGE))
		{
			Error("[ImageManager]: ssaoMap allocation failed !");
		}

		opts.sizeX = screen_width / 2;
		opts.sizeY = screen_height / 2;
		opts.format = IMF_R8;
		opts.usage = IMU_AORM;
		for (int i = 0; i < 2; ++i)
		{
			if (!globalImages.ssaoblur[i]->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
			{
				Error("[ImageManager]: ssaoBlur allocation failed !");
			}
		}

		opts.sizeX = screen_width;
		opts.sizeY = screen_height;
		opts.format = IMF_RGBA;
		opts.usage = IMU_DEFAULT;
		opts.srgb = true;
		if (!globalImages.defaultImage->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
		{
			Error("[ImageManager]: defaultImage allocation failed !");
		}


		opts.sizeX = screen_width;
		opts.sizeY = screen_height;
		opts.format = IMF_D24S8;
		opts.usage = IMU_DEPTH;
		opts.srgb = false;
		if (!globalImages.GBufferDepth->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
		{
			Error("[ImageManager]: Image defaultDepth allocation failed !");
		}
#if 0
		opts.format = IMF_D24S8;
		opts.usage = IMU_DEPTH;
		if (!globalImages.HDRdepth->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
		{
			Error("[ImageManager]: Image HDRdepth allocation failed !");
		}
#endif
		std::vector<unsigned int> tmp(16 * 16);
		opts.sizeX = 16;
		opts.sizeY = 16;
		opts.format = IMF_RGBA;
		opts.autocompress = false;
		opts.usage = IMU_DIFFUSE;

		// white
		tmp.assign(16 * 16, 0xFFFFFFFF);
		if (globalImages.whiteImage->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
		{
			globalImages.whiteImage->UpdateImageData(16, 16, 0, 0, 0, 0, tmp.data());
		}
		else
		{
			Error("[ImageManager]: Image whiteImage allocation failed !");
		}
		// black
		tmp.assign(16 * 16, 0xFF000000);
		if (globalImages.blackImage->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
		{
			globalImages.blackImage->UpdateImageData(16, 16, 0, 0, 0, 0, tmp.data());
		}
		else
		{
			Error("[ImageManager]: Image blackImage allocation failed !");
		}
		// gray
		tmp.assign(16 * 16, 0xFF7F7F7F);
		if (globalImages.grayImage->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
		{
			globalImages.grayImage->UpdateImageData(16, 16, 0, 0, 0, 0, tmp.data());
		}
		else
		{
			Error("[ImageManager]: Image grayImage allocation failed !");
		}
		// flat normal
		opts.usage = IMU_NORMAL;
		tmp.assign(16 * 16, 0xFFFF7F7F);
		if (globalImages.flatNormal->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
		{
			globalImages.flatNormal->UpdateImageData(16, 16, 0, 0, 0, 0, tmp.data());
		}
		else
		{
			Error("[ImageManager]: Image flatNormal allocation failed !");
		}

		opts.format = IMF_D32;
		opts.usage = IMU_DEPTH;
		opts.sizeX = renderGlobals.shadowResolution;
		opts.sizeY = renderGlobals.shadowResolution;
		opts.autocompress = false;
		opts.compressed = false;
		if (!globalImages.Shadow->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
		{
			Error("[ImageManager]: Image Shadow allocation failed !");
		}

		// SSAO Noise
		opts.format = IMF_RGBA16F;
		opts.usage = IMU_DEFAULT;
		opts.sizeX = 4;
		opts.sizeY = 4;
		opts.autocompress = false;
		opts.compressed = false;
		if (!globalImages.ssaoNoise->AllocImage(opts, IFL_NEAREST, IMR_REPEAT))
		{
			Error("[ImageManager]: Image SSAO noise allocation failed !");
		}

		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
		std::default_random_engine generator;
		std::vector<glm::vec4> ssaoNoise;
		for (unsigned int i = 0; i < 16; i++)
		{
			glm::vec4 noise(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				0.0f,
				0.0f);
			ssaoNoise.push_back(noise);
		}
		globalImages.ssaoNoise->UpdateImageData(4, 4, 0, 0, 0, 0, ssaoNoise.data());

		initialized = true;
		Image::Unbind();

		return true;
	}
	void ImageManager::Shutdown()
	{
		if (initialized)
		{
			initialized = false;
			for (auto* ptr : images) { delete ptr; }
		}
	}

	Image* ImageManager::LoadFromFile(std::string const& name, std::string const& filename, eImageUsage usage)
	{
		Image* newimg = AllocImage();
		if (newimg->Load(filename.c_str()))
		{
			newimg->SetUsage(usage);
			return newimg;
		}
		else
		{
			RemoveImage(newimg);
			return nullptr;
		}
	}
	void ImageManager::RemoveImage(int idx)
	{
		if (idx < images.size() && images[idx])
		{
			delete images[idx];
			images[idx] = nullptr;
			freelist.push_back(idx);
		}
	}

	void ImageManager::RemoveImage(Image* img)
	{
		if (img) RemoveImage(img->GetId());
	}

	Image* ImageManager::GetImage(int idx)
	{
		if (idx >= images.size()) return nullptr;

		return images[idx];
	}

	Image* ImageManager::GetImage(std::string const& name)
	{
		for (auto* e : images)
		{
			if (e->GetName() == name) return e;
		}
		return nullptr;
	}
	
	Image* ImageManager::AllocImage(std::string const& name)
	{
		if (!freelist.empty())
		{
			size_t idx = freelist.back();
			freelist.pop_back();
			images[idx] = new Image(name);
			images[idx]->id = idx;
			return images[idx];
		}
		
		images.emplace_back(new Image(name));
		const size_t idx = images.size() - 1;
		Image* res = images[idx];
		res->id = idx;

		return res;
	}
}