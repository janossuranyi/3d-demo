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
		for (int i = 0; i < STAGE_COUNT; ++i)
		{
			if (stages[i])
			{
				delete stages[i];
				stages[i] = nullptr;
			}
		}
	}

	stage_t* Material::GetStage(eStageType aType)
	{
		return stages[aType];
	}

	stage_t* Material::AllocStage(eStageType aType)
	{
		if (stages[aType])
		{
			return stages[aType];
		}
		
		stages[aType] = new stage_t;

		return stages[aType];
	}

	bool Material::IsEmpty() const
	{
		int active = 0;
		for (int i = 0; STAGE_COUNT; ++i)
		{
			if (stages[i]) ++active;
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

}