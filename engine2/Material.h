#pragma once
#include <string>
#include <vector>
#include <unordered_map>
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
		bool enabled;
		eStageType type;
		eShaderProg shader;
		glm::vec4 shaderParms[8];		// copied to uboUniforms_t.user01..07
		std::vector<Image*> images;
		eCoverage coverage;
		eCullMode cullMode;
		float alphaCutoff;

		void SetImage(int index, Image* image);
		stage_t();
	};

	class Material
	{
	public:
		Material();
		Material(const std::string& aName);
		~Material();
		stage_t& GetStage(eStageType aType);
		bool IsEmpty() const;
	private:
		std::string name;
		stage_t stages[STAGE_COUNT];
	};

	class MaterialManager
	{
	public:
		MaterialManager() = default;
		~MaterialManager();
		Material* CreateMaterial(const std::string& name);
		Material* FindMaterial(const std::string& name);
	private:
		std::vector<Material*> lstMaterial;
		std::unordered_map<std::string, Material*> mapMaterial;
	};
}