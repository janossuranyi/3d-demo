#pragma once

#include <GL/glew.h>
#include <string>

namespace jsr {

	enum eBufferTarget
	{
		BT_NONE,
		BT_VERTEX,
		BT_INDEX,
		BT_UNIFORM,
	};

	enum eBufferMapType
	{
		BM_READ,
		BM_WRITE
	};

	enum eBufferUsage
	{
		BU_STATIC,
		BU_DYNAMIC
	};

	extern void UnbindBufferObjects();

	class BufferObject
	{
	public:
		BufferObject::BufferObject();

		int GetSize() const
		{
			return size;
		}
		int GetOffset() const
		{
			return offset;
		}
		bool IsMapped() const
		{
			return buffer != nullptr;
		}
		bool IsView() const
		{
			return isview;
		}
		eBufferUsage GetUsage() const
		{
			return usage;
		}
		int GetAllocedSize() const
		{
			return (size + 15) & ~15;
		}
	public:
		bool AllocBufferObject(const void* data, int size, eBufferUsage usage);
		void FreeBufferObject();
		void Update(const void* data, int offset, int size);
		void* MapBuffer(eBufferMapType mapType);
		void UnmapBuffer();
		void MakeView(const BufferObject& other, int refOffset = 0, int refSize = 0);
		void Bind();
	private:
		void ClearWithoutFreeing()
		{
			offset = 0;
			buffer = 0;
			apiObject = 0xffff;
		}
	protected:
		eBufferTarget	target;
		eBufferUsage	usage;
		int				size;
		int				offset;
		bool			isview;
		// OpenGL
		GLuint			apiObject;
		void*			buffer;
	};

	class VertexBuffer : public BufferObject
	{
	public:
		VertexBuffer();
	};
	class IndexBuffer : public BufferObject
	{
	public:
		IndexBuffer();
	};
	class UniformBuffer : public BufferObject
	{
	public:
		UniformBuffer();
	};

}