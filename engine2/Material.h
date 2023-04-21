#pragma once
#include <string>
#include <glm/glm.hpp>
#include "./Image.h"

namespace jsr {

	enum eCoverage
	{
		COVERAGE_SOLID,
		COVERAGE_MASK,
		COVERAGE_BLEND
	};

	struct stage_t
	{

	};
	class Material
	{
	public:
		int id;
		std::string name;
		glm::vec4 shaderParms[8];
		Image const* texMRAO;
		Image const* texDiffuse;
		Image const* texNormal;
		Image const* texEmissive;
		Image const* texAO;	// if exists then same as texMRAO
		glm::vec4 kDiffuse;
		float kEmissive;
		float kMetallic;
		float kRoughness;
		float kAmbientOcclusion;
		float alphaCutoff;
		bool doubleSided;
		eCoverage coverage;
	};
}