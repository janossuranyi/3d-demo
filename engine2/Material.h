#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "./Image.h"
#include "./RenderProgs.h"
#include "./RenderCommon.h"

#define FLG_X_COVERAGE_SHIFT 0
#define FLG_X_COVERAGE_MASK 3
#define FLG_COVERAGE_SOLID 0
#define FLG_COVERAGE_MASKED 1
#define FLG_COVERAGE_BLEND 2

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
		STAGE_SHADOW,
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
		glm::vec4 diffuseScale;
		glm::vec4 emissiveScale;
		Image* images[IMU_COUNT];
		eCoverage coverage;
		eCullMode cullMode;
		float alphaCutoff;
		float roughnessScale;
		float metallicScale;
		void SetImage(int index, Image* image);
		stage_t();
	};

	class Material
	{
		friend class MaterialManager;
	public:
		Material();
		Material(const std::string& aName);
		~Material();
		stage_t& GetStage(eStageType aType);
		const stage_t& GetStage(eStageType aType) const;
		bool IsEmpty() const;
		int GetId() const;
		std::string GetName() const;
		void SetName(const std::string& name);
		bool IsValid() const;
	private:
		std::string name;
		int id;
		stage_t stages[STAGE_COUNT];
	};

	class MaterialManager
	{
	public:
		MaterialManager() = default;
		~MaterialManager();
		Material* CreateMaterial(const std::string& name);
		Material* FindMaterial(const std::string& name);
		void RemoveMaterial(Material* pM);
		Material* operator[](size_t index);
		Material* GetMaterial(int id);
	private:
		std::vector<Material*> materials;
		std::vector<int> freelist;
	};
}