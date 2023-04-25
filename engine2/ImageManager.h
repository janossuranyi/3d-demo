#pragma once

#include "./Image.h"

namespace jsr {

	struct internalImages_t
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
	private:

	};
}
