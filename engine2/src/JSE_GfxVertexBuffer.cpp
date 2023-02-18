#include "JSE.h"

JseVertexBuffer::JseVertexBuffer() :
	vtx_buffer_(),
	idx_buffer_(),
	vtx_offset_(0),
	vtx_size_(0),
	idx_offset_(0),
	idx_size_(0)
{
}

JseVertexBuffer::JseVertexBuffer(JseBufferID vertexBuf, int vtxOffset, int vtxSize) :
	vtx_buffer_(vertexBuf),
	vtx_offset_(vtxOffset),
	vtx_size_(vtxSize),
	idx_buffer_(),
	idx_offset_(0),
	idx_size_(0)
{
}

JseVertexBuffer::JseVertexBuffer(JseBufferID vertexBuf, int vtxOffset, int vtxSize, JseBufferID indexBuf, int idxOffset, int idxSize) :
	vtx_buffer_(vertexBuf),
	vtx_offset_(vtxOffset),
	vtx_size_(vtxSize),
	idx_buffer_(indexBuf),
	idx_offset_(idxOffset),
	idx_size_(idxSize)
{
}

JseBufferID JseVertexBuffer::vertexBuffer() const
{
	return vtx_buffer_;
}

JseBufferID JseVertexBuffer::indexBuffer() const
{
	return idx_buffer_;
}

int JseVertexBuffer::vertexOffset() const
{
	return vtx_offset_;
}

int JseVertexBuffer::indexOffset() const
{
	return idx_offset_;
}

