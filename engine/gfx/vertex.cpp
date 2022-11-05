#include "gfx/renderer.h"

namespace gfx {
	uint16_t VertexDecl::stride() const
	{
		return stride_;
	}
	bool VertexDecl::empty() const
	{
		return stride_ == 0;
	}

	VertexDecl& VertexDecl::begin()
	{ 
		attributes_.clear();
		stride_ = 0;
		return *this; 
	}

	VertexDecl& VertexDecl::end() { return *this; }

	const AttributeList& VertexDecl::attributes() const
	{
		return attributes_;
	}

	VertexDecl& VertexDecl::add(AttributeName name, AttributeType type, ushort count, bool normalized)
	{
		const int typeSize = getTypeSize(type);
		attributes_.emplace_back(VertexAttribute{ name,type,count, static_cast<uint>(stride_),normalized,0,0 });
		stride_ += typeSize * count;

		return *this;
	}

	VertexDecl& VertexDecl::addInstance(AttributeName name, AttributeType type, ushort count, bool normalized, uint offset, ushort binding, ushort divisor)
	{
		const int typeSize = getTypeSize(type);
		attributes_.emplace_back(VertexAttribute{ name,type,count,offset,normalized,divisor,binding });
		//stride_ += typeSize * count;

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
	
	uint16_t VertexDecl::getTypeSize(AttributeType type) const
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