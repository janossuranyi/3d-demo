#pragma once

#include "common.h"

struct Animation {

	enum class ePath { Translation, Rotation, Scale, Weights };
	enum class eInterpolation { Linear, Step, Cubic };
	struct Channel 
	{
		int		sampler;
		int		target;
		ePath	path;
	};

	struct Sampler 
	{
		eInterpolation		interpolation;
		int					input;
		std::vector<float>	output;
	};

	std::vector<Channel>					channels;
	std::vector<std::unique_ptr<Sampler>>	samplers;
	std::vector<std::vector<float>>			inputs;

	std::string								name;

	uint	addSampler();
	uint	addInput();
	uint	addChannel();

};