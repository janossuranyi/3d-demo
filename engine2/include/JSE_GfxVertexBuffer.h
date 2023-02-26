#ifndef JSE_VERTEX_BUFFER_H
#define JSE_VERTEX_BUFFER_H

namespace js {

	class VertexBuffer {
	public:
		VertexBuffer();
		VertexBuffer(JseBufferID vertexBuf, int vtxOffset, int vtxSize);
		VertexBuffer(JseBufferID vertexBuf, int vtxOffset, int vtxSize, JseBufferID indexBuf, int idxOffset, int idxSize);
		~VertexBuffer() {}
		template<typename T>
		int getBaseVertex() const {
			return vtx_offset_ / sizeof(T);
		}
		JseBufferID vertexBuffer() const;
		JseBufferID indexBuffer() const;
		int vertexOffset() const;
		int indexOffset() const;
	private:
		JseBufferID vtx_buffer_;
		JseBufferID idx_buffer_;
		int vtx_offset_;
		int vtx_size_;
		int idx_offset_;
		int idx_size_;
	};
}
#endif // !JSE_VERTEX_BUFFER_H

