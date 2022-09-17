#pragma once

#include "common.h"
#include "gpu_types.h"

struct Sampler
{

};
struct Image
{
	int					width;
	int					height;
	int					bits;
	int					component;
	eDataType			pixelType;
	std::vector<uchar>	image;
};

struct Texture
{
	enum class eType { D1, D2, D3, CUBE };

	eType				type;
	std::vector<uint>	images;

};