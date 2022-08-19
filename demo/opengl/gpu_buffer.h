#pragma once

#include <GL/glew.h>
#include <cinttypes>

enum class eGpuBufferTarget
{
	VERTEX, INDEX, UNIFORM, ENUM_SIZE
};

enum class eGpuBufferUsage
{
	STATIC, DYNAMIC, DEFAULT
} ;

enum eGpuBufferAccess
{
	MAP_READONLY, MAP_WRITEONLY, MAP_READWRITE, DEFAULT_ACCESSs
};

#define INVALID_BUFFER 0xFFFF

class GpuBuffer
{
public:
	GpuBuffer() = delete;
	~GpuBuffer();
	GpuBuffer(eGpuBufferTarget const type);
	GpuBuffer(const GpuBuffer&) = delete;
	GpuBuffer& operator=(GpuBuffer&) = delete;
	void bind() const;
	uint8_t* map(eGpuBufferAccess access);
	void unMap();
	bool isMapped() const;
	bool isCreated() const;
	bool create(uint32_t size, eGpuBufferUsage usage, const void* bytes = NULL);
	void reference(uint32_t offset, uint32_t size, GpuBuffer& ref);
	bool isOwnBuffer() const;
	void unBind() const;
	void bindVertexBuffer(uint32_t index, uint32_t offset, uint32_t stride);
private:
	GLuint mBuffer;
	bool mIsMapped;
	bool mIsReference;
	uint8_t* mMapPtr;
	uint32_t mSize;
	uint32_t mOffset;
	eGpuBufferTarget mTarget;
	eGpuBufferUsage mUsage;
	eGpuBufferAccess mAccess;

};

inline GpuBuffer::GpuBuffer(eGpuBufferTarget const type)
{
	mBuffer = INVALID_BUFFER;
	mTarget = type;   
	mSize = 0;
	mOffset = 0;
	mIsMapped = false;
	mIsReference = false;
	mMapPtr = NULL;
	mUsage = eGpuBufferUsage::STATIC;
	mAccess = DEFAULT_ACCESSs;
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
