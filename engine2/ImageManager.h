#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "./Image.h"

namespace jsr {

	struct globalImages_t
	{
		Image* HDRaccum;
		Image* HDRdepth;
		Image* GBufferFragPos;
		Image* GBufferAlbedo;
		Image* GBufferNormal;
		Image* GBufferSpec;
		Image* Shadow;
		Image* defaultImage;
		Image* defaultDepth;
		Image* whiteImage;
		Image* grayImage;
		Image* blackImage;
		Image* flatNormal;
	};

	class ImageManager
	{
	public:
		ImageManager();
		~ImageManager();
		bool Init();
		void Shutdown();
		Image* LoadFromFile(std::string const& name, std::string const& filename, eImageUsage usage);
		void RemoveImage(int idx);
		void RemoveImage(Image* img);
		Image* GetImage(int idx);
		Image* GetImage(std::string const& name);
		globalImages_t globalImages;
		Image* AllocImage(std::string const& name = "_default_");
	private:
		bool initialized;
		std::vector<Image*> images;
		std::vector<size_t> freelist;
	};
}
