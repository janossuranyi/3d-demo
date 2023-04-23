#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "./Image.h"
#include "./RenderProgs.h"
#include "./RenderCommon.h"

namespace jsr {

	enum eCoverage
	{
		COVERAGE_SOLID,
		COVERAGE_MASK,
		COVERAGE_BLEND
	};

	enum eStageType
	{
		STAGE_GBUFFER,
		STAGE_PRELIGHT,
		STAGE_POSTLIGHT,
		STAGE_PP,
		STAGE_DEBUG,
		STAGE_COUNT
	};

	struct stage_t
	{
		eStageType type;
		eShaderProg shader;
		glm::vec4 shaderParms[8];		// copied to uboUniforms_t.user01..07
		std::vector<Image*> images;
		eCoverage coverage;
		eCullMode cullMode;
		float alphaCutoff;

		void SetImage(int index, Image* image);
	};

	class Material
	{
	public:
		Material();
		Material(const std::string& aName);
		~Material();
		stage_t* GetStage(eStageType aType);
		stage_t* AllocStage(eStageType aType);
		bool IsEmpty() const;
	private:
		std::string name;
		stage_t* stages[STAGE_COUNT];
	};
}