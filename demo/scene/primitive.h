#pragma once

#include "common.h"
#include "material.h"
#include "gpu_types.h"
#include "bounds.h"

struct Primitive
{
	BoundingBox				aabb;
	std::vector<glm::vec3>	positions;
	std::vector<glm::vec3>	normals;
	std::vector<glm::vec3>	faceNormals;
	std::vector<glm::vec3>	colors0;
	std::vector<glm::vec3>	colors1;
	std::vector<glm::vec2>	texcoords0;
	std::vector<glm::vec2>	texcoords1;
	std::vector<glm::vec4>	tangents;
	std::vector<glm::uvec4>	joints;
	std::vector<glm::vec4>	weights;
	std::vector<uint>		indices;

	int						material{ -1 };
	eDrawMode				mode{ eDrawMode::TRIANGLES };
	
};