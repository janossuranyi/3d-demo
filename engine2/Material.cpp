#include "./Material.h"

namespace jsr {

	Material::Material() :
		name("_default_"),
		id(-1),
		stages() {}

	Material::Material(const std::string& aName):
		name(aName),
		id(-1),
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
	
	const stage_t& Material::GetStage(eStageType aType) const
	{
		return stages[aType];
	}

	bool Material::IsEmpty() const
	{
		int active = 0;
		for (int i = 0; i < STAGE_COUNT; ++i)
		{
			if (stages[i].enabled) ++active;
		}

		return active == 0;
	}

	int Material::GetId() const
	{
		return id;
	}

	void stage_t::SetImage(int index, Image* image)
	{
		if (index >= IMU_COUNT) return;

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
	
	std::string Material::GetName() const { return name; }

	void Material::SetName(const std::string& name)
	{
		this->name = name;
	}

	bool Material::IsValid() const
	{
		return id > -1;
	}

	MaterialManager::~MaterialManager()
	{
		for (auto* ptr : materials) { delete ptr; }
	}

	Material* MaterialManager::CreateMaterial(const std::string& name)
	{
		Material* res{};
		if (!freelist.empty())
		{
			size_t idx = freelist.back();
			freelist.pop_back();
			materials[idx] = new Material(name);
			materials[idx]->id = idx;
		}

		materials.emplace_back(new Material(name));
		res = materials.back();
		res->id = materials.size() - 1;

		return res;
	}

	Material* MaterialManager::FindMaterial(const std::string& name)
	{
		for (auto* e : materials)
		{
			if (e && e->GetName() == name)
			{
				return e;
			}
		}
		return nullptr;
	}

	void MaterialManager::RemoveMaterial(Material* pM)
	{
		if (!pM) return;

		for (int i = 0; i < materials.size(); ++i)
		{
			if (pM == materials[i])
			{
				delete materials[i];
				materials[i] = nullptr;
				freelist.push_back(i);
				break;
			}
		}
	}

	Material* MaterialManager::operator[](size_t index)
	{
		return GetMaterial((int)index);
	}

	Material* MaterialManager::GetMaterial(int id)
	{
		if (id > materials.size())
		{
			return nullptr;
		}

		return materials[id];
	}
}