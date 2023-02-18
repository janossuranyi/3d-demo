#pragma once
#include "engine/engine.h"
#include "math/math_converters.h"

using byte = uint8_t;

struct DrawVert
{
	struct hvec2 {
		halfFloat_t s, t;
	};
	struct unorm3 {
		byte x, y, z;
	};
	struct unorm4 {
		byte x, y, z, w;
	};

	vec3 position;		//12
	hvec2 texcoord;		//4
	unorm4 normal;		//4
	unorm4 tangent;		//4
	unorm4 color;		//4
	uint encoded;		//4 = 32

	DrawVert() :
		position{ 0.0f,0.0f,0.0f },
		texcoord{ 0,0 },
		normal{ 0,0,0,0 },
		tangent{ 0,0,0,0 },
		color{ 0,0,0,0 },
		encoded{ 0 } {}

	DrawVert& set_position(const float* apos);
	DrawVert& set_position(const glm::vec3& apos);
	DrawVert& set_texcoord(const float* apos);
	DrawVert& set_texcoord(const glm::vec2& apos);
	DrawVert& set_normal(const float* apos);
	DrawVert& set_normal(const glm::vec3& apos);
	DrawVert& set_tangent(const float* apos);
	DrawVert& set_tangent(const glm::vec4& apos);
	DrawVert& set_color(const float* apos);
	DrawVert& set_color(const glm::vec4& apos);
	DrawVert& set_encoded_data(uint val);
};

inline DrawVert& DrawVert::set_encoded_data(uint val)
{
	encoded = val;

	return *this;
}

inline DrawVert& DrawVert::set_tangent(const float* apos)
{
	tangent.x = VERTEX_FLOAT_TO_BYTE(apos[0]);
	tangent.y = VERTEX_FLOAT_TO_BYTE(apos[1]);
	tangent.z = VERTEX_FLOAT_TO_BYTE(apos[2]);
	tangent.w = VERTEX_FLOAT_TO_BYTE(apos[3]);

	return *this;
}

inline DrawVert& DrawVert::set_tangent(const glm::vec4& apos)
{
	tangent.x = VERTEX_FLOAT_TO_BYTE(apos[0]);
	tangent.y = VERTEX_FLOAT_TO_BYTE(apos[1]);
	tangent.z = VERTEX_FLOAT_TO_BYTE(apos[2]);
	tangent.w = VERTEX_FLOAT_TO_BYTE(apos[3]);

	return *this;
}

inline DrawVert& DrawVert::set_color(const float* apos)
{
	color.x = floatToUnorm8(apos[0]);
	color.y = floatToUnorm8(apos[1]);
	color.z = floatToUnorm8(apos[2]);
	color.w = floatToUnorm8(apos[3]);

	return *this;
}

inline DrawVert& DrawVert::set_color(const glm::vec4& apos)
{
	color.x = floatToUnorm8(apos[0]);
	color.y = floatToUnorm8(apos[1]);
	color.z = floatToUnorm8(apos[2]);
	color.w = floatToUnorm8(apos[3]);

	return *this;
}

inline DrawVert& DrawVert::set_normal(const float* apos)
{
	normal.x = VERTEX_FLOAT_TO_BYTE(apos[0]);
	normal.y = VERTEX_FLOAT_TO_BYTE(apos[1]);
	normal.z = VERTEX_FLOAT_TO_BYTE(apos[2]);
	normal.w = 0.0f;

	return *this;
}

inline DrawVert& DrawVert::set_normal(const glm::vec3& apos)
{
	normal.x = VERTEX_FLOAT_TO_BYTE(apos[0]);
	normal.y = VERTEX_FLOAT_TO_BYTE(apos[1]);
	normal.z = VERTEX_FLOAT_TO_BYTE(apos[2]);
	normal.w = 0.0f;

	return *this;
}

inline DrawVert& DrawVert::set_position(const float* apos)
{
	position.x = apos[0];
	position.y = apos[1];
	position.z = apos[2];
	//position.w = 1.0f;

	return *this;
}

inline DrawVert& DrawVert::set_position(const glm::vec3& apos)
{
	position.x = apos[0];
	position.y = apos[1];
	position.z = apos[2];
	//position.w = 1.0f;

	return *this;
}

inline DrawVert& DrawVert::set_texcoord(const float* apos)
{
	texcoord.s = F32toF16(apos[0]);
	texcoord.t = F32toF16(apos[1]);

	return *this;
}

inline DrawVert& DrawVert::set_texcoord(const glm::vec2& apos)
{
	texcoord.s = F32toF16(apos[0]);
	texcoord.t = F32toF16(apos[1]);

	return *this;
}

