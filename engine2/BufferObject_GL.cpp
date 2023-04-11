#include <GL/glew.h>
#include "./RenderBackend_GL.h"
#include "./BufferObject.h"
#include "./Logger.h"
#include <cassert>

namespace jsr {

	static const GLenum c_bufferUsageLut[] = { GL_STATIC_DRAW,GL_DYNAMIC_DRAW };
	static const GLenum c_bufferTargetLut[] = { 0, GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_UNIFORM_BUFFER };
	static const std::string c_bufferTargetNames[] = {"<none>", "VertexBuffer", "IndexBuffer", "UniformBuffer"};

	void UnbindBufferObjects()
	{
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	}

	bool BufferObject::AllocBufferObject(const void* data, int size, eBufferUsage usage)
	{
		this->size = size;
		this->usage = usage;

		if (apiObject != 0xffff) return false;

		bool allocationFailed = false;

		int numBytes = GetAllocedSize();
		GL_CHECK(glGenBuffers(1, &apiObject));
		if (apiObject == 0xffff)
		{
			Error("%s::AllocBufferObject: failed !!", c_bufferTargetNames[target]);
			return false;
		}

		GL_CHECK(glBindBuffer(c_bufferTargetLut[target], apiObject));
		glBufferData(c_bufferTargetLut[target], numBytes, nullptr, c_bufferUsageLut[usage]);
		GLenum err = glGetError();
		if (err == GL_OUT_OF_MEMORY)
		{
			Error("%s::AllocBufferObject: allocation failed", c_bufferTargetNames[target]);
			allocationFailed = true;
		}

		Update(data, 0, numBytes);

		return allocationFailed;
	}

	void BufferObject::FreeBufferObject()
	{
		if (IsMapped())
		{
			UnmapBuffer();
		}
		if (IsView())
		{
			ClearWoFreeing();
			return;
		}
		if (apiObject == 0xffff)
		{
			return;
		}
		GL_CHECK(glDeleteBuffers(1, &apiObject));
	}

	void BufferObject::Update(const void* data, int offset, int updateSize)
	{
		assert((GetOffset() & 15) == 0);
		assert(apiObject != 0xffff);

		if (!data)
		{
			return;
		}

		if (updateSize > GetSize())
		{
			Error("%s::Update: size overrun %i > %i", c_bufferTargetNames[target], updateSize, GetSize());
		}

		const int numbytes = updateSize; // (updateSize + 15) & ~15;
		if (buffer != nullptr)
		{
			memcpy((char*)buffer + offset, data, numbytes);
		}
		else
		{
			GL_CHECK(glBindBuffer(c_bufferTargetLut[target], apiObject));
			GL_CHECK(glBufferSubData(c_bufferTargetLut[target], GetOffset() + offset, numbytes, data));
		}
	}

	void* BufferObject::MapBuffer(eBufferMapType mapType)
	{
		assert(apiObject != 0xffff);
		assert(!IsMapped());

		buffer = NULL;
		GL_CHECK(glBindBuffer(c_bufferTargetLut[target], apiObject));
		if (mapType == BM_READ)
		{
			GL_CHECK(buffer = glMapBufferRange(c_bufferTargetLut[target], 0, GetAllocedSize(), GL_MAP_READ_BIT | GL_MAP_UNSYNCHRONIZED_BIT));
			if (buffer != NULL)
			{
				buffer = (char*)buffer + GetOffset();
			}
		}
		else if (mapType == BM_WRITE)
		{
			GL_CHECK(buffer = glMapBufferRange(c_bufferTargetLut[target], 0, GetAllocedSize(), GL_MAP_WRITE_BIT /*| GL_MAP_INVALIDATE_RANGE_BIT*/ | GL_MAP_UNSYNCHRONIZED_BIT));
			if (buffer != NULL)
			{
				buffer = (char*)buffer + GetOffset();
			}
		}
		else
		{
			assert(false);
		}

		if (buffer == NULL)
		{
			Error("%s::MapBuffer: failed", c_bufferTargetNames[target]);
		}

		return buffer;
	}

	void BufferObject::UnmapBuffer()
	{
		assert(apiObject != 0xffff);
		assert(IsMapped());
		GL_CHECK(glBindBuffer(c_bufferTargetLut[target], apiObject));
		if (!glUnmapBuffer(c_bufferTargetLut[target]))
		{
			Error("%s::UnmapBuffer: failed", c_bufferTargetNames[target]);
		}
		buffer = NULL;
	}

	void BufferObject::MakeView(const BufferObject& other, int refOffset, int refSize)
	{
		assert(IsMapped() == false);
		assert(refOffset >= 0);
		assert(refSize >= 0);
		assert(refOffset + refSize <= other.GetSize());
		FreeBufferObject();

		size = refSize;
		offset = other.GetOffset() + refOffset;
		usage = other.usage;
		apiObject = other.apiObject;
		isview = true;
	}

}

