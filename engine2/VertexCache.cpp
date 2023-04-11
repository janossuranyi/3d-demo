#include <cmath>
#include "./VertexCache.h"

namespace jsr {

	VertexCache::VertexCache()
	{		
		staticCacheSize = DEFAULT_MAX_STATIC_CACHE;
		transientCacheSize = DEFAULT_MAX_TRANSIENT_CACHE;
		staticUboCacheSize = 4 * 1024 * 1024;
		transientUboCacheSize = 64 * 1024;
		uniformBufferAligment = 256;
		uniformBufferAligmentBits = 8;
	}
	VertexCache::~VertexCache()
	{
		Shutdown();
	}

	static void ClearGeoBufferSet(geoBufferSet_t& gbs)
	{
		gbs.indexAlloced.store(0, std::memory_order_relaxed);
		gbs.vertexAlloced.store(0, std::memory_order_relaxed);
		gbs.uniformsAlloced.store(0, std::memory_order_relaxed);
	}

	static void AllocBufferSet(geoBufferSet_t& gbs, int vertexSize, int indexSize, int uboSize, eBufferUsage usage)
	{
		gbs.vertexBuffer.AllocBufferObject(nullptr, vertexSize, usage);
		gbs.indexBuffer.AllocBufferObject(nullptr, indexSize, usage);
		gbs.uniformBuffer.AllocBufferObject(nullptr, uboSize, usage);
		ClearGeoBufferSet(gbs);
	}

	static void MapBufferSet(geoBufferSet_t& gbs)
	{
		if (gbs.vertexPtr == nullptr)
		{
			gbs.vertexPtr = (byte*)gbs.vertexBuffer.MapBuffer(BM_WRITE);
		}
		if (gbs.indexPtr == nullptr)
		{
			gbs.indexPtr = (byte*)gbs.indexBuffer.MapBuffer(BM_WRITE);
		}
		if (gbs.uniformsPtr == nullptr)
		{
			gbs.uniformsPtr = (byte*)gbs.uniformBuffer.MapBuffer(BM_WRITE);
		}
	}
	static void UnmapBufferSet(geoBufferSet_t& gbs)
	{
		if (gbs.vertexPtr != nullptr)
		{
			gbs.vertexPtr = nullptr;
			gbs.vertexBuffer.UnmapBuffer();
		}
		if (gbs.indexPtr != nullptr)
		{
			gbs.indexPtr = nullptr;
			gbs.indexBuffer.UnmapBuffer();
		}
		if (gbs.uniformsPtr != nullptr)
		{
			gbs.uniformsPtr = nullptr;
			gbs.uniformBuffer.UnmapBuffer();
		}
	}
	void VertexCache::Init(int uniformBufferAligment_)
	{
		if (initialized) return;

		assert((uniformBufferAligment_ & (uniformBufferAligment_ - 1)) == 0);
		uniformBufferAligment = uniformBufferAligment_;
		uniformBufferAligmentBits = static_cast<int>(std::floor(std::log2(uniformBufferAligment_)));

		for (int k = 0; k < JSE_VERTEX_CACHE_FRAMES; ++k)
		{
			AllocBufferSet(transientBufferSet[k], transientCacheSize, transientCacheSize, transientUboCacheSize, BU_DYNAMIC);
		}
		AllocBufferSet(staticBufferSet, staticCacheSize, staticCacheSize, staticUboCacheSize, BU_STATIC);
		MapBufferSet(transientBufferSet[activeFrame]);
		MapBufferSet(staticBufferSet);
		
		initialized = true;
	}

	void VertexCache::Shutdown()
	{
		if (!initialized) return;

		for (int k = 0; k < JSE_VERTEX_CACHE_FRAMES; ++k) {
			transientBufferSet[k].vertexBuffer.FreeBufferObject();
			transientBufferSet[k].indexBuffer.FreeBufferObject();
			transientBufferSet[k].uniformBuffer.FreeBufferObject();
		}
		staticBufferSet.vertexBuffer.FreeBufferObject();
		staticBufferSet.indexBuffer.FreeBufferObject();
		staticBufferSet.uniformBuffer.FreeBufferObject();
		initialized = false;
	}

	void VertexCache::Restart()
	{
		Shutdown();
		Init();
	}

	void VertexCache::Frame()
	{
		++activeFrame;
		UnmapBufferSet(transientBufferSet[listNum]);
		UnmapBufferSet(staticBufferSet);

		renderFrame = listNum;
		listNum = activeFrame % JSE_VERTEX_CACHE_FRAMES;
		MapBufferSet(transientBufferSet[listNum]);
		ClearGeoBufferSet(transientBufferSet[listNum]);
	}

	vertCacheHandle_t VertexCache::AllocStaticVertex(int vertexCount, int vertexSize)
	{
		vertCacheHandle_t r = (1 << JSE_VERTEX_CACHE_STATIC_SHIFT) | RealAllocVertex(staticBufferSet, vertexCount * vertexSize);

		return r;
	}

	vertCacheHandle_t VertexCache::AllocStaticIndex(int indexCount, int indexSize)
	{
		vertCacheHandle_t r = (1 << JSE_VERTEX_CACHE_STATIC_SHIFT) | RealAllocIndex(staticBufferSet, indexCount * indexSize);

		return r;
	}

	vertCacheHandle_t VertexCache::AllocTransientVertex(int vertexCount, int vertexSize)
	{
		vertCacheHandle_t r = RealAllocVertex(transientBufferSet[listNum], vertexCount * vertexSize) | ((uint64)activeFrame & ((1 << JSE_VERTEX_CACHE_FRAME_BITS) - 1)) << JSE_VERTEX_CACHE_FRAME_SHIFT;

		return r;
	}

	vertCacheHandle_t VertexCache::AllocTransientIndex(int indexCount, int indexSize)
	{
		vertCacheHandle_t r = RealAllocIndex(transientBufferSet[listNum], indexCount * indexSize) | ((uint64)activeFrame & ((1 << JSE_VERTEX_CACHE_FRAME_BITS) - 1)) << JSE_VERTEX_CACHE_FRAME_SHIFT;

		return r;
	}

	vertCacheHandle_t VertexCache::AllocStaticUniform(int size)
	{
		vertCacheHandle_t r = (1 << JSE_VERTEX_CACHE_STATIC_SHIFT) | RealAllocUniform(staticBufferSet, size);

		return r;
	}

	vertCacheHandle_t VertexCache::AllocTransientUniform(int size)
	{
		vertCacheHandle_t r = RealAllocUniform(transientBufferSet[listNum], size) | ((uint64)activeFrame & ((1 << JSE_VERTEX_CACHE_FRAME_BITS) - 1)) << JSE_VERTEX_CACHE_FRAME_SHIFT;

		return r;
	}

	bool VertexCache::GetVertexBuffer(vertCacheHandle_t handle, VertexBuffer& dest)
	{
		const bool		isStatic = (handle >> JSE_VERTEX_CACHE_STATIC_SHIFT) & 1;
		const int		offset = int((handle >> JSE_VERTEX_CACHE_OFFSET_SHIFT) & ((1 << JSE_VERTEX_CACHE_OFFSET_BITS) - 1));
		const int		size = int((handle >> JSE_VERTEX_CACHE_SIZE_SHIFT) & ((1 << JSE_VERTEX_CACHE_SIZE_BITS) - 1));
		const int		frameNum = int((handle >> JSE_VERTEX_CACHE_FRAME_SHIFT) & ((1 << JSE_VERTEX_CACHE_FRAME_BITS) - 1));

		if (isStatic)
		{
			dest.MakeView(staticBufferSet.vertexBuffer, offset << 4, size);
			return true;
		}
		if (frameNum != ((activeFrame - 1) & ((1 << JSE_VERTEX_CACHE_FRAME_BITS) - 1)))
		{
			return false;
		}
		dest.MakeView(transientBufferSet[renderFrame].vertexBuffer, offset << 4, size);
	}

	bool VertexCache::GetIndexBuffer(vertCacheHandle_t handle, IndexBuffer& dest)
	{
		const bool		isStatic = (handle >> JSE_VERTEX_CACHE_STATIC_SHIFT) & 1;
		const int		offset = int((handle >> JSE_VERTEX_CACHE_OFFSET_SHIFT) & ((1 << JSE_VERTEX_CACHE_OFFSET_BITS) - 1));
		const int		size = int((handle >> JSE_VERTEX_CACHE_SIZE_SHIFT) & ((1 << JSE_VERTEX_CACHE_SIZE_BITS) - 1));
		const int		frameNum = int((handle >> JSE_VERTEX_CACHE_FRAME_SHIFT) & ((1 << JSE_VERTEX_CACHE_FRAME_BITS) - 1));

		if (isStatic)
		{
			dest.MakeView(staticBufferSet.indexBuffer, offset << 4, size);
			return true;
		}
		if (frameNum != ((activeFrame - 1) & ((1 << JSE_VERTEX_CACHE_FRAME_BITS) - 1)))
		{
			return false;
		}
		dest.MakeView(transientBufferSet[renderFrame].indexBuffer, offset << 4, size);
	}

	bool VertexCache::GetUniformBuffer(vertCacheHandle_t handle, UniformBuffer& dest)
	{
		const bool		isStatic = (handle >> JSE_VERTEX_CACHE_STATIC_SHIFT) & 1;
		const int		offset = int((handle >> JSE_VERTEX_CACHE_OFFSET_SHIFT) & ((1 << JSE_VERTEX_CACHE_OFFSET_BITS) - 1));
		const int		size = int((handle >> JSE_VERTEX_CACHE_SIZE_SHIFT) & (1 << JSE_VERTEX_CACHE_SIZE_BITS - 1));
		const int		frameNum = int((handle >> JSE_VERTEX_CACHE_FRAME_SHIFT) & ((1 << JSE_VERTEX_CACHE_FRAME_BITS) - 1));

		if (isStatic)
		{
			dest.MakeView(staticBufferSet.uniformBuffer, offset << uniformBufferAligmentBits, size);
			return true;
		}
		if (frameNum != ((activeFrame - 1) & ((1 << JSE_VERTEX_CACHE_FRAME_BITS) - 1)))
		{
			return false;
		}
		dest.MakeView(transientBufferSet[renderFrame].uniformBuffer, offset << uniformBufferAligmentBits, size);
	}

	vertCacheHandle_t VertexCache::RealAllocVertex(geoBufferSet_t& gbs, int size)
	{
		int _size = (size + 15) & ~15;
		int offset = gbs.vertexAlloced.fetch_add(_size, std::memory_order_relaxed);
		offset >>= 4;

		vertCacheHandle_t r = ((uint64)offset << JSE_VERTEX_CACHE_OFFSET_SHIFT) | ((uint64)size << JSE_VERTEX_CACHE_SIZE_SHIFT);

		return r;
	}

	vertCacheHandle_t VertexCache::RealAllocUniform(geoBufferSet_t& gbs, int size)
	{

		int align = uniformBufferAligment - 1;
		int _size = (size + align) & ~align;
		int offset = gbs.uniformsAlloced.fetch_add(_size, std::memory_order_relaxed);
		offset >>= uniformBufferAligmentBits;

		vertCacheHandle_t r = ((uint64)offset << JSE_VERTEX_CACHE_OFFSET_SHIFT) | ((uint64)size << JSE_VERTEX_CACHE_SIZE_SHIFT);

		return r;
	}

	vertCacheHandle_t VertexCache::RealAllocIndex(geoBufferSet_t& gbs, int size)
	{
		int _size = (size + 15) & ~15;
		int offset = gbs.indexAlloced.fetch_add(_size, std::memory_order_relaxed);
		offset >>= 4;

		vertCacheHandle_t r = ((uint64)offset << JSE_VERTEX_CACHE_OFFSET_SHIFT) | ((uint64)size << JSE_VERTEX_CACHE_SIZE_SHIFT);

		return r;
	}

}
