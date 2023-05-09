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
		void SetValue(void* ptr);
		inline eEntityType GetType() const		{ return type; }
		inline Light* GetLight() const			{ return reinterpret_cast<Light*>( value ); }
		inline RenderModel* GetModel() const	{ return reinterpret_cast<RenderModel*>( value ); }
	private: 
		eEntityType type;
		void* value;
	};
}
