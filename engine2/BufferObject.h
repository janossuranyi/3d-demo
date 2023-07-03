#pragma once

#include <GL/glew.h>
#include <string>
#include "./EngineTypes.h"

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
		BM_READ = 1,
		BM_WRITE = 2,
		BM_COHERENT = 4,
		BM_PERSISTENT = 8
	};

	enum eBufferUsage
	{
		BU_STATIC,
		BU_DYNAMIC
	};

	extern void UnbindBufferObjects();

	class BufferObject
	{
		friend class VertexCache;
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
		bool AllocBufferObject(const void* data, int size, eBufferUsage usage, int mapType = BM_READ|BM_WRITE);
		void FreeBufferObject();
		void Update(const void* data, int offset, int size);
		void* MapBuffer(int mapType);
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
		friend class VertexCache;
	public:
		VertexBuffer();
		void BindVertexBuffer(int binding, uint32 offset, uint32 stride) const;
	};
	class IndexBuffer : public BufferObject
	{
		friend class VertexCache;
	public:
		IndexBuffer();
		void BindIndexBuffer() const;
	};
	class UniformBuffer : public BufferObject
	{
		friend class VertexCache;
		friend class ProgramManager;
	public:
		UniformBuffer();
	};

}