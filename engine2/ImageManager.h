#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "./Image.h"

namespace jsr {

	struct globalImages_t
	{
		Image* HDRaccum;
		Image* GBufferAlbedo;
		Image* GBufferNormal;
		Image* GBufferSpec;
		Image* Depth32;
		Image* whiteImage;
		Image* flatNormal;
	};

	class ImageManager
	{
	public:
		ImageManager();
		~ImageManager();
		bool Init();
		void Shutdown();
		int LoadFromFile(std::string const& name, std::string const& filename, eImageUsage usage);
		int AddImage(Image* img);
		void RemoveImage(int idx);
		void RemoveImage(std::string const& name);
		Image* GetImage(int idx);
		Image* GetImage(std::string const& name);
		globalImages_t globalImages;
	private:
		size_t AllocImage();
		bool initialized;
		std::vector<Image*> images;
		std::unordered_map<std::string, Image*> imagemap;
	};
}
