#ifndef SCENE_LIGHT_H
#define SCENE_LIGHT_H

#include "engine/gfx/renderer.h"

namespace scene {
	struct Light
	{
		String id;
		gfx::LightType type;

		vec3 color;
		float power;		
		float range;
		float innerConeAngle;
		float outerConeAngle;
	};
}

#endif // !SCENE_LIGHT_H
