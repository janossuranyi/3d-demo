#pragma once

#include <cinttypes>
#include "math/math_converters.h"
#include "renderer.h"


namespace gfx {

	using byte = uint8_t;

	struct vec4 {
		float x, y, z, w;
	};
	struct hvec2 {
		halfFloat_t s,t;
	};
	struct unorm {
		byte x, y, z, w;
	};

	template<VertexDeclType DeclTy>
	struct VertexDecl {};

	using DrawVert = VertexDecl<VertexDeclType::Draw>;
	
	template<>
	struct VertexDecl<VertexDeclType::Draw>
	{
		vec4 position;		//16
		hvec2 texcoord;		//4
		unorm normal;		//4
		unorm tangent;		//4
		unorm color;		//4 = 32

		DrawVert& set_position(const float* apos);
		DrawVert& set_texcoord(const float* apos);
		DrawVert& set_normal(const float* apos);
		DrawVert& set_tangent(const float* apos);
		DrawVert& set_color(const float* apos);
	};

	inline DrawVert& VertexDecl<VertexDeclType::Draw>::set_tangent(const float* apos)
	{
		tangent.x = VERTEX_FLOAT_TO_BYTE(apos[0]);
		tangent.y = VERTEX_FLOAT_TO_BYTE(apos[1]);
		tangent.z = VERTEX_FLOAT_TO_BYTE(apos[2]);
		tangent.w = VERTEX_FLOAT_TO_BYTE(apos[3]);

		return *this;
	}

	inline DrawVert& VertexDecl<VertexDeclType::Draw>::set_color(const float* apos)
	{
		color.x = floatToUnorm8(apos[0]);
		color.y = floatToUnorm8(apos[1]);
		color.z = floatToUnorm8(apos[2]);
		color.w = floatToUnorm8(apos[3]);

		return *this;
	}

	inline DrawVert& VertexDecl<VertexDeclType::Draw>::set_normal(const float* apos)
	{
		normal.x = VERTEX_FLOAT_TO_BYTE(apos[0]);
		normal.y = VERTEX_FLOAT_TO_BYTE(apos[1]);
		normal.z = VERTEX_FLOAT_TO_BYTE(apos[2]);
		normal.w = VERTEX_FLOAT_TO_BYTE(apos[3]);

		return *this;
	}

	inline DrawVert& VertexDecl<VertexDeclType::Draw>::set_position(const float* apos)
	{
		position.x = apos[0];
		position.y = apos[1];
		position.z = apos[2];
		position.w = apos[3];

		return *this;
	}

	inline DrawVert& VertexDecl<VertexDeclType::Draw>::set_texcoord(const float* apos)
	{
		texcoord.s = apos[0];
		texcoord.t = apos[1];

		return *this;
	}
}
