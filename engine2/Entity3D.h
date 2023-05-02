#pragma once

namespace jsr {

	enum eEntityType
	{
		ENT_MODEL,
		ENT_LIGHT,
		ENT_LIGHT_PROBE,
		ENT_ENV_PROBE,
		ENT_COUNT
	};

	class RenderModel;
	class Light;
	class Entity3D 
	{
	public:
		Entity3D(eEntityType type);
		~Entity3D() = default;
		inline eEntityType GetType() const { return type; }
		inline Light const* GetLight() const { return light; }
		inline RenderModel const* GetModel() const { return model; }
	private: 
		eEntityType type;
		const RenderModel* model;
		const Light* light;
	};
}
