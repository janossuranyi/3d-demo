#pragma once

#include "common.h"
#include "gpu_types.h"

struct Sampler
{
	eTexMinFilter	minFilter;
	eTexMagFilter	magFilter;
	eTexWrap		wrapS;	
	eTexWrap		wrapT;

	Sampler() :
		minFilter(eTexMinFilter::LINEAR),
		magFilter(eTexMagFilter::LINEAR),
		wrapS(eTexWrap::REPEAT),
		wrapT(eTexWrap::REPEAT) {}
};

struct Image
{
	int					width;
	int					height;
	int					bits;
	int					component;
	eDataType			pixelType;
	std::vector<uchar>	image;

	Image() :
		width(0),
		height(0),
		bits(0),
		component(0),
		pixelType(eDataType::BYTE) {}

	Image(Image&) = delete;
	Image(Image&&) = delete;
	Image& operator=(Image&) = delete;
	Image& operator=(Image&&) = delete;
};

struct Texture
{
	enum class eType { D1, D2, D3, CUBE };

	eType	type;
	int		image;
	int		sampler;

	Texture() :
		type(eType::D2),
		image(-1),
		sampler(-1) {}

};