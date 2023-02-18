#pragma once

#include "common.h"
#include "gpu_types.h"

struct Sampler
{
	FilterMin	minFilter;
	FilterMag	magFilter;
	Wrap		wrapS;	
	Wrap		wrapT;

	Sampler() :
		minFilter(FilterMin::LINEAR),
		magFilter(FilterMag::LINEAR),
		wrapS(Wrap::REPEAT),
		wrapT(Wrap::REPEAT) {}
};

struct Image
{
	int					width;
	int					height;
	int					bits;
	int					component;
	ComponentType		pixelType;
	std::vector<uchar>	image;
	std::string			name;

	Image() :
		width(0),
		height(0),
		bits(0),
		component(0),
		name("<empty>"),
		pixelType(ComponentType::BYTE) {}

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