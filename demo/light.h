#pragma once
#include <memory>
#include <string>
#include "types.h"

struct SpotLight
{
	SpotLight() :
		innerConeAngle(0.0f),
		outerConeAngle(0.7853981634f) {}

	float innerConeAngle;
	float outerConeAngle;
};

struct Light
{
	enum Type { DIRECTIONAL, POINT, SPOT };
	using Ptr = std::shared_ptr<Light>;

	Light(Type type_) :
		type(type_),
		m_id(-1),
		color{1.0f,1.0f,1.0f,1.0f},
		range(0.0f) {}

	int id() const { return m_id; }
	void setId(int id) { m_id = id; }
	std::string name;
	Type type;
	SpotLight spot;
	vec4 color;
	float range;		// attenuation = max( min( 1.0 - ( current_distance / range )4, 1 ), 0 ) / current_distance2
	int m_id;

	inline void setColor(float r, float g, float b, float i)
	{
		color.r = r;
		color.g = g;
		color.b = b;
		color.a = i;
	}
};