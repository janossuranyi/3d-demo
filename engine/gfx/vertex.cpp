#include "gfx/renderer.h"

namespace gfx {

	bool VertexDecl::empty() const
	{
		return attributes_.empty();
	}

	VertexDecl& VertexDecl::begin()
	{ 
		attributes_.clear();
		return *this; 
	}

	VertexDecl& VertexDecl::end() { return *this; }

	VertexDecl& VertexDecl::reset_offset()
	{
		offset_ = 0;

		return *this;
	}

	const AttributeList& VertexDecl::attributes() const
	{
		return attributes_;
	}

	VertexDecl& VertexDecl::add(AttributeName name, AttributeType type, ushort count, bool normalized, ushort stride, ushort binding, ushort divisor)
	{
		const ushort size = getTypeSize(type);
		attributes_.emplace_back(VertexAttribute{ name,type,count,offset_,normalized,divisor,binding,stride });
		offset_ += size * count;

		return *this;
	}

	void VertexDecl::setHandle(VertexLayoutHandle handle)
	{
		handle_ = handle;
	}

	VertexLayoutHandle VertexDecl::handle() const
	{
		return handle_;
	}
	
	ushort VertexDecl::getTypeSize(AttributeType type)
	{
		switch (type) {
		case AttributeType::Byte: 
		case AttributeType::UByte:
			return sizeof(char);
		case AttributeType::Short:
		case AttributeType::UShort:
			return sizeof(short);
		case AttributeType::Int:
		case AttributeType::UInt:
			return sizeof(int);
		case AttributeType::Half:
			return sizeof(short);
		case AttributeType::Float:
			return sizeof(float);
		}
	}
}