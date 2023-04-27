#include <algorithm>
#include "./Logger.h"
#include "./ImageManager.h"
#include "./Image.h"
#include "./RenderSystem.h"

namespace jsr {
	ImageManager::ImageManager() :
		initialized(false),
		globalImages()
	{}

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
		globalImages.GBufferAlbedo = new Image("_gbuffer_albedo");
		globalImages.GBufferNormal = new Image("_gbuffer_normal");
		globalImages.Depth32 = new Image("_depth");
		globalImages.defaultDepth = new Image("_defaultDepth");
		globalImages.defaultImage = new Image("_defaultImage");
		globalImages.GBufferSpec = new Image("_gbuffer_specular");
		globalImages.HDRaccum = new Image("_hdrimage");
		globalImages.HDRdepth = new Image("_hdrDepth");
		globalImages.whiteImage = new Image("_white");
		globalImages.grayImage = new Image("_gray");
		globalImages.flatNormal = new Image("_flatnorm");


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
		if (!globalImages.GBufferAlbedo->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
		{
			Error("[ImageManager]: Image GBufferAlbedo allocation failed !");
		}
		AddImage(globalImages.GBufferAlbedo);

		opts.format = IMF_RG16F;
		opts.usage = IMU_NORMAL;
		if (!globalImages.GBufferNormal->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
		{
			Error("[ImageManager]: Image GBufferNormal allocation failed !");
		}
		AddImage(globalImages.GBufferNormal);

		opts.format = IMF_D32;
		opts.usage = IMU_DEPTH;
		if (!globalImages.Depth32->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
		{
			Error("[ImageManager]: Image GBufferNormal allocation failed !");
		}
		AddImage(globalImages.Depth32);

		opts.format = IMF_RGBA;
		opts.usage = IMU_AORM;
		if (!globalImages.GBufferSpec->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
		{
			Error("[ImageManager]: Image GBufferSpec allocation failed !");
		}
		AddImage(globalImages.GBufferSpec);

		opts.format = IMF_R11G11B10F;
		opts.usage = IMU_HDR;
		if (!globalImages.HDRaccum->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
		{
			Error("[ImageManager]: Image HDRaccum allocation failed !");
		}
		AddImage(globalImages.HDRaccum);


		opts.format = IMF_RGBA;
		opts.usage = IMU_DEFAULT;
		if (!globalImages.defaultImage->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
		{
			Error("[ImageManager]: defaultImage allocation failed !");
		}
		AddImage(globalImages.defaultImage);

		opts.format = IMF_D32;
		opts.usage = IMU_DEPTH;
		if (!globalImages.defaultDepth->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
		{
			Error("[ImageManager]: Image defaultDepth allocation failed !");
		}
		AddImage(globalImages.defaultDepth);

		opts.format = IMF_D32;
		opts.usage = IMU_DEPTH;
		if (!globalImages.HDRdepth->AllocImage(opts, IFL_LINEAR, IMR_CLAMP_TO_EDGE))
		{
			Error("[ImageManager]: Image HDRdepth allocation failed !");
		}
		AddImage(globalImages.HDRdepth);

		initialized = true;
		Image::Unbind();

		return true;
	}
	void ImageManager::Shutdown()
	{
		if (initialized)
		{
			initialized = false;
			/*
			delete globalImages.GBufferAlbedo;
			delete globalImages.GBufferNormal;
			delete globalImages.Depth32;
			delete globalImages.GBufferSpec;
			delete globalImages.HDRaccum;
			delete globalImages.whiteImage;
			delete globalImages.flatNormal;
			*/
		}

		for (auto* img : images) { delete img; }

		images.clear();
		imagemap.clear();
	}
	int ImageManager::LoadFromFile(std::string const& name, std::string const& filename, eImageUsage usage)
	{
		Image* newimg = new Image(name);
		if (newimg->Load(filename.c_str()))
		{
			newimg->SetUsage(usage);
			size_t idx = AllocImage();
			images[idx] = newimg;

			if (imagemap.count(name) == 0)
			{
				imagemap.emplace(name, newimg);
			}
			else
			{
				Info("[ImageManager]: name (%s) already exists", name.c_str());
			}
			return idx;
		}
		else
		{
			delete newimg;
			return -1;
		}
	}
	int ImageManager::AddImage(Image* img)
	{
		size_t idx = AllocImage();
		images[idx] = img;
		if (imagemap.count(img->GetName()) == 0)
		{
			imagemap.emplace(img->GetName(), img);
		}
		else
		{
			Info("[ImageManager]: name (%s) already exists", img->GetName().c_str());
		}
		return (int)idx;
	}
	void ImageManager::RemoveImage(int idx)
	{
		Image* tmp{};
		std::swap(images[idx], tmp);
		imagemap.erase(tmp->GetName());
		delete tmp;
	}
	void ImageManager::RemoveImage(std::string const& name)
	{
		auto it = imagemap.find(name);
		if (std::end(imagemap) != it)
		{
			Image* what = it->second;
			for (int i = 0; i < images.size(); ++i)
			{
				if (images[i] == what && images[i] != nullptr)
				{
					delete images[i];
					images[i] = nullptr;
				}
			}
			imagemap.erase(name);
		}
	}
	Image* ImageManager::GetImage(int idx)
	{
		assert(idx < images.size());
		return images[idx];
	}
	Image* ImageManager::GetImage(std::string const& name)
	{
		auto it = imagemap.find(name);
		if (std::end(imagemap) != it)
		{
			return it->second;
		}
		return nullptr;
	}
	size_t ImageManager::AllocImage()
	{
		for (int i = 0; i < images.size(); ++i)
		{
			if (images[i] == nullptr)
			{
				return i;
			}
		}
		
		images.emplace_back(nullptr);
		return images.size() - 1;
	}
}