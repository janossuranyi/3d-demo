#pragma once

#include <cinttypes>
#include <string>
#include <vector>
//#include "renderer.h"

namespace gfx {

	enum class AttributeType { Byte, UnsignedByte, Short, UnsignedShort, Int, UnsignedInt, Half, Float };
	enum class AttributeName { Position, TexCoord0, Normal, Tangent, Color0, TexCoord1, TexCoord2, TexCoord3, Color1, Color2, Color3};

	struct VertexAttribute
	{
		AttributeName name;
		AttributeType type;
		uint16_t count;
		char* offset;
		bool normalized;
	};

	using AttributeList = std::vector<VertexAttribute>;

	class VertexDecl_t
	{
	public:

		~VertexDecl_t() = default;

		uint16_t stride() const;
		bool empty() const;
		VertexDecl_t& begin();
		VertexDecl_t& end();
		const AttributeList& attributes() const;
		VertexDecl_t& add(AttributeName name, AttributeType type, uint16_t count, bool normalized);

	private:
		uint16_t getTypeSize(AttributeType type) const;
		uint16_t stride_;
		AttributeList attributes_;
	};
}
