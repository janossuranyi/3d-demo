#include "vertex.h"

namespace gfx {
	uint16_t VertexDecl_t::stride() const
	{
		return stride_;
	}
	bool VertexDecl_t::empty() const
	{
		return stride_ == 0;
	}

	VertexDecl_t& VertexDecl_t::begin()
	{ 
		attributes_.clear();
		stride_ = 0;
		return *this; 
	}

	VertexDecl_t& VertexDecl_t::end() { return *this; }

	const AttributeList& VertexDecl_t::attributes() const
	{
		return attributes_;
	}

	VertexDecl_t& VertexDecl_t::add(const std::string& name, AttributeType type, uint16_t count, bool normalized)
	{
		const int typeSize = getTypeSize(type);
		attributes_.emplace_back(VertexAttribute{ name,type,count, reinterpret_cast<char*>(static_cast<uintptr_t>(stride_)),normalized });
		stride_ += typeSize * count;

		return *this;
	}
	
	uint16_t VertexDecl_t::getTypeSize(AttributeType type) const
	{
		switch (type) {
		case AttributeType::Byte: 
		case AttributeType::UnsignedByte:
			return sizeof(char);
		case AttributeType::Short:
		case AttributeType::UnsignedShort:
			return sizeof(short);
		case AttributeType::Int:
		case AttributeType::UnsignedInt:
			return sizeof(int);
		case AttributeType::Half:
			return sizeof(short);
		case AttributeType::Float:
			return sizeof(float);
		}
	}
}