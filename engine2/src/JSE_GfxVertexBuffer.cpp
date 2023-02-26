#include "JSE.h"

namespace js {

	VertexBuffer::VertexBuffer() :
		vtx_buffer_(),
		idx_buffer_(),
		vtx_offset_(0),
		vtx_size_(0),
		idx_offset_(0),
		idx_size_(0)
	{
	}

	VertexBuffer::VertexBuffer(JseBufferID vertexBuf, int vtxOffset, int vtxSize) :
		vtx_buffer_(vertexBuf),
		vtx_offset_(vtxOffset),
		vtx_size_(vtxSize),
		idx_buffer_(),
		idx_offset_(0),
		idx_size_(0)
	{
	}

	VertexBuffer::VertexBuffer(JseBufferID vertexBuf, int vtxOffset, int vtxSize, JseBufferID indexBuf, int idxOffset, int idxSize) :
		vtx_buffer_(vertexBuf),
		vtx_offset_(vtxOffset),
		vtx_size_(vtxSize),
		idx_buffer_(indexBuf),
		idx_offset_(idxOffset),
		idx_size_(idxSize)
	{
	}

	JseBufferID VertexBuffer::vertexBuffer() const
	{
		return vtx_buffer_;
	}

	JseBufferID VertexBuffer::indexBuffer() const
	{
		return idx_buffer_;
	}

	int VertexBuffer::vertexOffset() const
	{
		return vtx_offset_;
	}

	int VertexBuffer::indexOffset() const
	{
		return idx_offset_;
	}

}