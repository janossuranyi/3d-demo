#pragma once

#include "common.h"
#include "gpu_vertex_layout.h"

struct Primitive
{
	uint vertexCount;
	uint indexCount;
	
	std::vector<glm::vec3> aPositions;
	std::vector<glm::vec3> aNormals;
	std::vector<glm::vec3> aColors0;
	std::vector<glm::vec3> aColors1;
	std::vector<glm::vec2> aTexcoords0;
	std::vector<glm::vec2> aTexcoords1;
	std::vector<glm::vec4> aTangents;



};