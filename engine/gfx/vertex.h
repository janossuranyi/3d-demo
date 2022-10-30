#pragma once

#include <cinttypes>
#include <string>
#include <vector>
//#include "renderer.h"

namespace gfx {

	enum class AttributeType { Byte, UnsignedByte, Short, UnsignedShort, Int, UnsignedInt, Half, Float };

	struct VertexAttribute
	{
		std::string name;
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
		VertexDecl_t& add(const std::string& name, AttributeType type, uint16_t count, bool normalized);

	private:
		uint16_t getTypeSize(AttributeType type) const;
		uint16_t stride_;
		AttributeList attributes_;
	};
}
