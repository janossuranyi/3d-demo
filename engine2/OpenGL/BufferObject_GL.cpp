#include <cassert>
#include <GL/glew.h>
#include "engine2/EngineTypes.h"
#include "./RenderBackend_GL.h"
#include "engine2/RenderBackend.h"
#include "engine2/BufferObject.h"
#include "engine2/Logger.h"

#define JSR_GL_IMMUTABLE_STORAGE 1

namespace jsr {

	static const GLenum c_bufferUsageLut[] = { GL_STATIC_DRAW,GL_DYNAMIC_DRAW };
	static const GLenum c_bufferTargetLut[] = { GL_ZERO, GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_UNIFORM_BUFFER };
	static const std::string c_bufferTargetNames[] = {"<none>", "VertexBuffer", "IndexBuffer", "UniformBuffer"};

	void UnbindBufferObjects()
	{
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	}

	bool BufferObject::AllocBufferObject(const void* data, int size, eBufferUsage usage, int mapType)
	{
		this->size = size;
		this->usage = usage;

		if (apiObject != 0xffff) return false;

		bool allocationFailed = false;

		int numBytes = GetAllocedSize();
		GL_CHECK(glCreateBuffers(1, &apiObject));
		if (apiObject == 0xffff)
		{
			Error("%s::AllocBufferObject: failed !!", c_bufferTargetNames[target]);
			return false;
		}
#if JSR_GL_IMMUTABLE_STORAGE
		GLbitfield flags = GL_DYNAMIC_STORAGE_BIT;
		if (mapType & BM_READ)			flags |= GL_MAP_READ_BIT;
		if (mapType & BM_WRITE)			flags |= GL_MAP_WRITE_BIT;
		if (mapType & BM_PERSISTENT)	flags |= GL_MAP_PERSISTENT_BIT;
		if (mapType & BM_COHERENT)		flags |= GL_MAP_COHERENT_BIT;

		glNamedBufferStorage(apiObject, numBytes, data, flags);
#else
		GL_CHECK(glNamedBufferData(apiObject, numBytes, data, c_bufferUsageLut[usage]));
#endif
		GLenum err = glGetError();
		if (err == GL_OUT_OF_MEMORY)
		{
			Error("%s::AllocBufferObject: allocation failed", c_bufferTargetNames[target]);
			allocationFailed = true;
		}

		//Update(data, 0, numBytes);

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
			ClearWithoutFreeing();
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

		const int numbytes = (updateSize + 15) & ~15;
		if (buffer != nullptr)
		{
			memcpy((char*)buffer + GetOffset() + offset, data, numbytes);
		}
		else
		{
			//GL_CHECK(glBindBuffer(c_bufferTargetLut[target], apiObject));
			GL_CHECK(glNamedBufferSubData(apiObject, GetOffset() + offset, numbytes, data));
		}
	}

	void* BufferObject::MapBuffer(int mapType)
	{
		assert(apiObject != 0xffff);
		assert(!IsMapped());

		buffer = NULL;
#if JSR_GL_IMMUTABLE_STORAGE
		GLbitfield flags{};
		if (mapType & BM_READ)			flags |= GL_MAP_READ_BIT;
		if (mapType & BM_WRITE)			flags |= GL_MAP_WRITE_BIT;
		if (mapType & BM_PERSISTENT)	flags |= GL_MAP_PERSISTENT_BIT;
		if (mapType & BM_COHERENT)		flags |= GL_MAP_COHERENT_BIT;

		GL_CHECK(buffer = glMapNamedBufferRange(apiObject, 0, GetAllocedSize(), flags));
#else
		if (mapType & BM_READ)
		{
			GL_CHECK(buffer = glMapNamedBufferRange(apiObject, 0, GetAllocedSize(), GL_MAP_READ_BIT | GL_MAP_UNSYNCHRONIZED_BIT));
			if (buffer != NULL)
			{
				buffer = (char*)buffer + GetOffset();
			}
		}
		else if (mapType == BM_WRITE)
		{
			GL_CHECK(buffer = glMapNamedBufferRange(apiObject, 0, GetAllocedSize(), GL_MAP_WRITE_BIT /*| GL_MAP_INVALIDATE_RANGE_BIT*/ | GL_MAP_UNSYNCHRONIZED_BIT));
			if (buffer != NULL)
			{
				buffer = (char*)buffer + GetOffset();
			}
		}
		else
		{
			assert(false);
		}
#endif
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
		//GL_CHECK(glBindBuffer(c_bufferTargetLut[target], apiObject));
		if (!glUnmapNamedBuffer(apiObject))
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
		buffer = other.buffer;
		isview = true;
	}

	void BufferObject::Bind()
	{
		GL_CHECK(glBindBuffer(c_bufferTargetLut[target], apiObject));
	}

	void VertexBuffer::BindVertexBuffer(int binding, uint32 offset, uint32 stride) const
	{
		auto& vtxbind = glcontext.vtxBindings[binding];
		if (vtxbind.buffer != apiObject || vtxbind.offset != offset || vtxbind.stride != stride)
		{
			GL_CHECK(glBindVertexBuffer(binding, apiObject, offset, stride));
			vtxbind.buffer = apiObject;
			vtxbind.offset = offset;
			vtxbind.stride = stride;
		}
	}

	void IndexBuffer::BindIndexBuffer() const
	{
		if (glcontext.currentIndexBuffer != apiObject)
		{
			GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, apiObject));
			glcontext.currentIndexBuffer = apiObject;
		}
	}
}

