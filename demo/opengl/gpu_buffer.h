#pragma once

#include <GL/glew.h>
#include <cinttypes>

#include "gpu_types.h"
#include "types.h"

class GpuBuffer
{
	friend class Pipeline;
	friend class GPU;
public:

	/** Empty constructor initializes a VERTEX buffer
	*/
	GpuBuffer() :mTarget(BufferTarget::VERTEX) {}

	/** Destroys the underlying gpu buffer
	*/
	~GpuBuffer();

	/** Initialize a specific buffer target
	*/
	GpuBuffer(BufferTarget const type);

	/** Copy constructor disabled
	*/
	GpuBuffer(const GpuBuffer&) = delete;
	
	/** Copy assign disabled
	*/
	GpuBuffer& operator=(GpuBuffer&) = delete;

	GpuBuffer(GpuBuffer&& moved) = delete;

	GpuBuffer& operator=(GpuBuffer&& moved) = delete;

	uint8_t* map(unsigned int access);

	uint8_t* mapPeristentWrite();

	uint8_t* mappedAddress() const { return mMapPtr; }

	void unMap();

	bool isMapped() const;

	bool isCreated() const;

	bool create(uint32_t size, BufferUsage usage, unsigned int accesFlags, const void* bytes = NULL);

	void reference(uint32_t offset, uint32_t size, GpuBuffer& ref);

	bool isOwnBuffer() const;

	void unBind() const;

	void bindVertexBuffer(uint32_t stream, uint32_t offset, uint32_t stride) const;

	void bindIndexed(uint32_t index, uint32_t offset = 0, uint32_t size = 0);

	void updateSubData(uint32_t offset, uint32_t size, const void* data);
private:
	void		move(GpuBuffer&);
	GLuint		mBuffer;
	bool		mIsMapped;
	bool		mIsReference;
	uint8_t*	mMapPtr;
	uint32_t	mSize;
	uint32_t	mOffset;
	BufferTarget mTarget;
	BufferUsage	mUsage;
	GLbitfield	mAccess;

};

inline GpuBuffer::GpuBuffer(BufferTarget const type)
{
	mBuffer = INVALID_BUFFER;
	mTarget = type;   
	mSize = 0;
	mOffset = 0;
	mIsMapped = false;
	mIsReference = false;
	mMapPtr = NULL;
	mUsage = BufferUsage::STATIC;
	mAccess = BA_MAP_WRITE;
}

inline bool GpuBuffer::isMapped() const
{ 
	return mIsMapped; 
}

inline bool GpuBuffer::isCreated() const
{
	return mBuffer != INVALID_BUFFER;
}
inline bool GpuBuffer::isOwnBuffer() const
{
	return !mIsReference;
}
