#ifndef JSE_GFX_DRAW_VERT_H
#define JSE_GFX_DRAW_VERT_H

#include "JSE_Converters.h"

namespace js {

	struct GfxDrawVert {
		vec3 xyz;
		vec2 uv;
		vec3 ntc;

		void encode_normal(vec3 x);
		void encode_tangent(vec4 x);
		void encode_color(vec4 x);
	};

	inline void GfxDrawVert::encode_normal(vec3 x)
	{
		ntc.x = glm::uintBitsToFloat(pack32(
			floatToUnorm8((x.x + 1.0f) / 2.0f),
			floatToUnorm8((x.y + 1.0f) / 2.0f),
			floatToUnorm8((x.z + 1.0f) / 2.0f),
			0
		));
	}

	inline void GfxDrawVert::encode_tangent(vec4 x)
	{
		ntc.y = glm::uintBitsToFloat(pack32(
			floatToUnorm8((x.x + 1.0f) / 2.0f),
			floatToUnorm8((x.y + 1.0f) / 2.0f),
			floatToUnorm8((x.z + 1.0f) / 2.0f),
			floatToUnorm8((x.w + 1.0f) / 2.0f)
		));
	}

	inline void GfxDrawVert::encode_color(vec4 x) {
		ntc.z = glm::uintBitsToFloat(pack32(
			floatToUnorm8(x.x),
			floatToUnorm8(x.y),
			floatToUnorm8(x.z),
			floatToUnorm8(x.w)
		));
	}

}

#endif // !JSE_DRAW_VERT_H

