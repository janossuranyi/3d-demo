#include <cassert>
#include <GL/glew.h>
#include "gpu.h"
#include "gpu_buffer.h"
#include "gpu_utils.h"
#include "gpu_types.h"
#include "logger.h"
/*
OpenGL buffer implementation
*/

/*
GpuBuffer::GpuBuffer(GpuBuffer&& moved) noexcept
{
	move(moved);
}

GpuBuffer& GpuBuffer::operator=(GpuBuffer&& moved) noexcept
{
	move(moved);
	return *this;
}
*/

void GpuBuffer::unBind() const
{
	const GLenum target = GL_CastBufferType(mTarget);
	GL_CHECK(glBindBuffer(target, 0));
}

void GpuBuffer::bindVertexBuffer(uint32_t stream, uint32_t offset, uint32_t stride) const
{
	GL_CHECK(glBindVertexBuffer(stream, mBuffer, offset, stride));
}

void GpuBuffer::bindIndexed(uint32_t index, uint32_t offset, uint32_t pixelByteSize)
{
	const GLenum target = GL_CastBufferType(mTarget);
	if (pixelByteSize == 0)
	{
		GL_CHECK(glBindBufferBase(target, index, mBuffer));
	}
	else
	{
		GL_CHECK(glBindBufferRange(target, index, mBuffer, offset, pixelByteSize));
	}
}

void GpuBuffer::updateSubData(uint32_t offset, uint32_t pixelByteSize, const void* data)
{
	GL_CHECK(glBufferSubData(GL_CastBufferType(mTarget), GLintptr(offset), GLsizeiptr(pixelByteSize), data));
}

void GpuBuffer::move(GpuBuffer& moved)
{
	mBuffer = moved.mBuffer;
	mIsMapped = moved.mIsMapped;
	mIsReference = moved.mIsReference;
	mMapPtr = moved.mMapPtr;
	mSize = moved.mSize;
	mOffset = moved.mOffset;
	mTarget = moved.mTarget;
	mUsage = moved.mUsage;
	mAccess = moved.mAccess;

	moved.mBuffer = INVALID_BUFFER;
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
	return map((BA_MAP_WRITE | BA_MAP_PERSISTENT | BA_MAP_COHERENT) & mAccess);
}

bool GpuBuffer::create(uint32_t pixelByteSize, BufferUsage usage, unsigned int accessFlags, const void* bytes)
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
	case BufferUsage::STATIC:
		bu = GL_STATIC_DRAW;
		break;
	case BufferUsage::DYNAMIC:
		bu = GL_DYNAMIC_DRAW;
		break;
	}

	uint32_t real_size = pixelByteSize;
	if (mTarget == BufferTarget::VERTEX)
	{
		real_size = (pixelByteSize + VERTEX_BUFFER_ALIGN) & ~(VERTEX_BUFFER_ALIGN);
	}
	else if (mTarget == BufferTarget::UNIFORM)
	{
		real_size = (pixelByteSize + UNIFORM_BUFFER_ALIGN) & ~(UNIFORM_BUFFER_ALIGN);
	}
	else
	{
		real_size = (pixelByteSize + INDEX_BUFFER_ALIGN) & ~(INDEX_BUFFER_ALIGN);
	}

	GL_CHECK(glBindBuffer(target, mBuffer));
	if (access)
	{
		GL_CHECK(glBufferStorage(target, real_size, nullptr, access));
	}
	else
	{
		GL_CHECK(glBufferData(target, real_size, nullptr, bu));
	}

	if (bytes)
	{
		glBufferSubData(target, 0, pixelByteSize, bytes);
	}

	mSize = pixelByteSize;
	mAccess = accessFlags;

	Info("Buffer %d, type: %d allocated, size: %d bytes.", mBuffer, mTarget, pixelByteSize);
	

	return glGetError() == GL_NO_ERROR;
}

void GpuBuffer::reference(uint32_t offset, uint32_t pixelByteSize, GpuBuffer& ref)
{
	assert(mBuffer != INVALID_BUFFER);
	assert(mIsReference == false);
	assert((offset + pixelByteSize) < mSize);

	ref.mAccess = mAccess;
	ref.mIsReference = true;
	ref.mBuffer = mBuffer;
	ref.mIsMapped = mIsMapped;
	ref.mMapPtr = mMapPtr;
	ref.mOffset = offset;
	ref.mSize = pixelByteSize;

	Info("Buffer %d, type: %d Referenced.", mBuffer, mTarget);
}

GpuBuffer::~GpuBuffer()
{
	if (isOwnBuffer() && mBuffer != INVALID_BUFFER)
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
