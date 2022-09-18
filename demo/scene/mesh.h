#pragma once

#include "common.h"
#include "primitive.h"
#include "bounds.h"

struct Mesh
{
	typedef std::vector<std::unique_ptr<Primitive>> primitiveArray_t;

	BoundingBox				aabb;
	primitiveArray_t		primitives;
	std::vector<uint>		weights;
	std::string				name;

	inline uint addPrimitive()
	{
		primitives.emplace_back();
		primitives.back().reset(new Primitive());

		return primitives.size() - 1;
	}

};