#ifndef SCENE_LIGHT_H
#define SCENE_LIGHT_H

#include "engine/gfx/renderer.h"
#include <cmath>

namespace scene {
	struct Light
	{
		gfx::LightType type;

		vec3 color;
		float power;
		float range;
		float innerConeAngle;
		float outerConeAngle;
		float diffuseScale;
		float specularScale;
		vec3 position;
		vec3 target;

		Light() :
			type(gfx::LightType::Point),
			power(1.0f),
			range(1.0f),
			innerConeAngle(45.0f * M_PI / 180.f),
			outerConeAngle(45.0f * M_PI / 180.f),
			diffuseScale(1.f),
			specularScale(1.f),
			color(1.f),
			position(vec3(0.f,1.f,0.f)),
			target(0.f)
		{}
	};
}

#endif // !SCENE_LIGHT_H
