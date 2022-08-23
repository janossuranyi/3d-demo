#include <cassert>
#include <GL/glew.h>
#include "gpu_buffer.h"
#include "gpu_utils.h"
#include "logger.h"
/*
OpenGL buffer implementation
*/

static inline GLenum GL_CastBufferType(eGpuBufferTarget type)
{
	switch (type)
	{
	case eGpuBufferTarget::VERTEX:
		return GL_ARRAY_BUFFER;
	case eGpuBufferTarget::INDEX:
		return GL_ELEMENT_ARRAY_BUFFER;
	case eGpuBufferTarget::UNIFORM:
		return GL_UNIFORM_BUFFER;
	}

	return GL_FALSE;
}

void GpuBuffer::bind() const
{
	assert(mBuffer != INVALID_BUFFER);

	const GLenum target = GL_CastBufferType(mTarget);
	GL_CHECK(glBindBuffer(target, mBuffer));	
	
}

void GpuBuffer::unBind() const
{
	const GLenum target = GL_CastBufferType(mTarget);
	GL_CHECK(glBindBuffer(target, 0));
}

void GpuBuffer::bindVertexBuffer(uint32_t stream, uint32_t offset, uint32_t stride) const
{
	GL_CHECK(glBindVertexBuffer(stream, mBuffer, offset, stride));
}

void GpuBuffer::bindIndexed(uint32_t index, uint32_t offset, uint32_t size)
{
	const GLenum target = GL_CastBufferType(mTarget);
	if (size == 0)
	{
		GL_CHECK(glBindBufferBase(target, index, mBuffer));
	}
	else
	{
		GL_CHECK(glBindBufferRange(target, index, mBuffer, offset, size));
	}
}

void GpuBuffer::unMap()
{
	assert(mBuffer != INVALID_BUFFER);
	assert(mIsMapped);
	const GLenum target = GL_CastBufferType(mTarget);

	GL_CHECK(glBindBuffer(target, mBuffer));
	GL_CHECK(glUnmapBuffer(target));
	mIsMapped = false;
	mMapPtr = nullptr;

	Info("Buffer %d, type: %d UnMapped.", mBuffer, mTarget);

}

uint8_t* GpuBuffer::map(unsigned int pAccess)
{
	assert(mIsMapped == false);
	assert(mBuffer != INVALID_BUFFER);
	assert(mIsReference == false);
	
	const GLenum target = GL_CastBufferType(mTarget);

	const GLbitfield access = GL_castBufferAccessFlags(pAccess);
		
	uint8_t* ptr = nullptr;

	GL_CHECK(glBindBuffer(target, mBuffer));
	GL_CHECK(ptr = static_cast<uint8_t*>(glMapBufferRange(target, 0, mSize, access)));
	GL_CHECK(glBindBuffer(target, 0));

	if (ptr)
	{
		mMapPtr = ptr;
		mIsMapped = true;
		Info("Buffer %d, type: %d mapped.", mBuffer, mTarget);
	}
	else
	{
		Info("Buffer %d, type: %d map failed!.", mBuffer, mTarget);
	}

	return ptr;
}

uint8_t* GpuBuffer::mapPeristentWrite()
{
	return map(BA_MAP_WRITE | BA_MAP_PERSISTENT | BA_MAP_COHERENT);
}

bool GpuBuffer::create(uint32_t size, eGpuBufferUsage usage, unsigned int accessFlags, const void* bytes)
{
	assert(mBuffer == INVALID_BUFFER);
	assert(mIsReference == false);
	assert(mIsMapped == false);

	if (bytes)
	{
		assert(((uintptr_t) bytes & 15) == 0);
	}

	const GLenum target = GL_CastBufferType(mTarget);
	const GLbitfield access = GL_castBufferAccessFlags(accessFlags);

	GL_CHECK(glGenBuffers(1, &mBuffer));

	GLenum bu = GL_STATIC_DRAW;
	switch (usage)
	{
	case eGpuBufferUsage::STATIC:
		bu = GL_STATIC_DRAW;
		break;
	case eGpuBufferUsage::DYNAMIC:
		bu = GL_DYNAMIC_DRAW;
		break;
	}

	if (mTarget == eGpuBufferTarget::VERTEX) size = (size + 15) & ~(15);
	else size = (size + 15) & ~(15);

	GL_CHECK(glBindBuffer(target, mBuffer));
	if (access)
	{
		GL_CHECK(glBufferStorage(target, size, bytes, access));
	}
	else
	{
		GL_CHECK(glBufferData(target, size, bytes, bu));
	}
	mSize = size;

	Info("Buffer %d, type: %d allocated, size: %d bytes.", mBuffer, mTarget, size);
	

	return glGetError() == GL_NO_ERROR;
}

void GpuBuffer::reference(uint32_t offset, uint32_t size, GpuBuffer& ref)
{
	assert(mBuffer != INVALID_BUFFER);
	assert(mIsReference == false);
	assert((offset + size) < mSize);

	ref.mAccess = mAccess;
	ref.mIsReference = true;
	ref.mBuffer = mBuffer;
	ref.mIsMapped = mIsMapped;
	ref.mMapPtr = mMapPtr;
	ref.mOffset = offset;
	ref.mSize = size;

	Info("Buffer %d, type: %d Referenced.", mBuffer, mTarget);
}

GpuBuffer::~GpuBuffer()
{
	if (isOwnBuffer())
	{
		if (isMapped())
		{
			unMap();
		}
		const GLenum target = GL_CastBufferType(mTarget);
		GL_CHECK(glBindBuffer(target, 0));
		GL_CHECK(glDeleteBuffers(1, &mBuffer));
		Info("Buffer %d, type: %d deleted", mBuffer, mTarget);

		mSize = 0;
		mOffset = 0;
		mBuffer = INVALID_BUFFER;
		mMapPtr = nullptr;
	}

}
