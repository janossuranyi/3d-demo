#include "./Material.h"

namespace jsr {

	Material::Material() :
		name("_default_"),
		stages() {}

	Material::Material(const std::string& aName):
		name(aName),
		stages()
	{
	}

	Material::~Material()
	{
	}

	stage_t& Material::GetStage(eStageType aType)
	{
		return stages[aType];
	}

	bool Material::IsEmpty() const
	{
		int active = 0;
		for (int i = 0; STAGE_COUNT; ++i)
		{
			if (stages[i].enabled) ++active;
		}

		return active == 0;
	}

	void stage_t::SetImage(int index, Image* image)
	{
		if (images.size() <= index)
		{
			images.resize(index + 1);
		}
		images[index] = image;
		image->AddRef();
	}

	stage_t::stage_t() :
		enabled(false),
		type(STAGE_GBUFFER),
		shader(PRG_DEFERRED_GBUFFER_MR),
		shaderParms(),
		images(),
		coverage(COVERAGE_SOLID),
		cullMode(CULL_NONE),
		alphaCutoff(0.5f)
	{
	}
	Material* MaterialManager::CreateMaterial(const std::string& name)
	{
		Material* res = new Material();
		lstMaterial.push_back(res);
		mapMaterial.emplace(name, res);

		return res;
	}
}