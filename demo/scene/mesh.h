#pragma once

#include "common.h"
#include "primitive.h"
#include "bounds.h"

struct Mesh
{
	BoundingBox				aabb;
	std::vector<uint>		primitives;
	std::vector<uint>		weights;
	std::string				name;

};