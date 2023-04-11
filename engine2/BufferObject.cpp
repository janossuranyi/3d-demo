
#include "./BufferObject.h"

namespace jsr {
	
	BufferObject::BufferObject()
	{
		size = 0;
		offset = 0;
		isview = false;
		usage = BU_STATIC;
		apiObject = 0;
		buffer = nullptr;
		target = BT_NONE;
		apiObject = 0xffff;
	}

	VertexBuffer::VertexBuffer() : BufferObject()
	{
		target = BT_VERTEX;
	}

	IndexBuffer::IndexBuffer()
	{
		target = BT_INDEX;
	}

	UniformBuffer::UniformBuffer()
	{
		target = BT_UNIFORM;
	}
}