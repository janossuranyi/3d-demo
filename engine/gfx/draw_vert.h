#ifndef GFX_DRAW_VERT_H
#define GFX_DRAW_VERT_H

#include <glm/glm.hpp>
#include <cmath>
#include "engine/common.h"
#include "engine/math/math_converters.h"

namespace gfx {

	struct DrawVert
	{
		vec3 in_Position;
		vec2 in_TexCoord;
		vec3 in_NTC;		// Normal,Tangent,Color as packed 8bit RGBA

		void setPosition(const vec3& position);
		void setTexCoord(const vec2& texCoord);
		void setNormalTangentColor(const vec3& N, const vec4& T, const vec4& C);
		void setNormal(const vec3& N);
		void setTangent(const vec4& T);
		void setColor(const vec4& C);
		vec3 normal() const;
		vec4 tangent() const;
		vec4 color() const;
		vec3 position() const;
		vec2 texCoord() const;
	};

	static_assert(sizeof(DrawVert) == 32, "DrawVert should be 32 byte");

	inline void DrawVert::setPosition(const vec3& position)
	{
		in_Position = position;
	}

	inline void DrawVert::setTexCoord(const vec2& texCoord)
	{
		in_TexCoord = texCoord;
	}

	inline void DrawVert::setNormalTangentColor(const vec3& N, const vec4& T, const vec4& C)
	{
		setNormal(N);
		setTangent(T);
		setColor(C);
	}
	inline void DrawVert::setNormal(const vec3& N)
	{
		in_NTC.x = glm::uintBitsToFloat(pack32(
			floatToUnorm8((N.x + 1.0f) / 2.0f),
			floatToUnorm8((N.y + 1.0f) / 2.0f),
			floatToUnorm8((N.z + 1.0f) / 2.0f), 0
		));
	}
	inline void DrawVert::setTangent(const vec4& T)
	{
		in_NTC.y = glm::uintBitsToFloat(pack32(
			floatToUnorm8((T.x + 1.0f) / 2.0f),
			floatToUnorm8((T.y + 1.0f) / 2.0f),
			floatToUnorm8((T.z + 1.0f) / 2.0f),
			floatToUnorm8(std::clamp(T.w, 0.0f, 1.0f))
		));
	}
	inline void DrawVert::setColor(const vec4& C)
	{
		in_NTC.z = glm::uintBitsToFloat(pack32(
			floatToUnorm8(C.x),
			floatToUnorm8(C.y),
			floatToUnorm8(C.z),
			floatToUnorm8(C.w)
		));
	}
	inline vec3 DrawVert::normal() const
	{
		vec4 tmp = unpackR8G8G8A8(glm::floatBitsToUint(in_NTC.x));

		return vec3(tmp);
	}
	inline vec4 DrawVert::tangent() const
	{
		const vec4 a = unpackR8G8G8A8(glm::floatBitsToUint(in_NTC.y));
		const vec3 b = (vec3(a) * 2.0f) - vec3(1.0f);
		const float w = (std::floorf((a.w * 255.100006103515625f) / 128.0f) * 2.0f) - 1.0f;

		return vec4(b, w);
	}
	inline vec4 DrawVert::color() const
	{
		return unpackR8G8G8A8(glm::floatBitsToUint(in_NTC.z));
	}
	inline vec3 DrawVert::position() const
	{
		return in_Position;
	}
	inline vec2 DrawVert::texCoord() const
	{
		return in_TexCoord;
	}
}
#endif // !GFX_DRAW_VERT_H
