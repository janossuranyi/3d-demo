#include <cmath>
#include "./VertexCache.h"
#include "./Logger.h"

#define DEFAULT_MAX_STATIC_CACHE	(128*1024*1024)
#define DEFAULT_MAX_TRANSIENT_CACHE (16*1024*1024)

#define CACHE_OFFSET_BITS	27
#define CACHE_SIZE_BITS		24	// 16MB
#define CACHE_FRAME_BITS	10
#define CACHE_STATIC_SHIFT	0
#define CACHE_OFFSET_SHIFT	1
#define CACHE_SIZE_SHIFT	28
#define CACHE_FRAME_SHIFT	52

#define CACHE_OFFSET_MASK	((1ULL << CACHE_OFFSET_BITS) - 1)
#define CACHE_SIZE_MASK		((1ULL << CACHE_SIZE_BITS) - 1)
#define CACHE_FRAME_MASK	((1ULL << CACHE_FRAME_BITS) - 1)
#define CACHE_STATIC		(1)

namespace jsr {

	VertexCache::VertexCache()
	{		
		staticCacheSize			= DEFAULT_MAX_STATIC_CACHE;
		transientCacheSize		= DEFAULT_MAX_TRANSIENT_CACHE;
		staticUboCacheSize		= 4 * 1024 * 1024;
		transientUboCacheSize	= 8 * 1024 * 1024;
		uniformBufferAligment	= 256;
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
	bool VertexCache::Init(int uniformBufferAligment_)
	{
		if (initialized) return false;

		assert((uniformBufferAligment_ & (uniformBufferAligment_ - 1)) == 0);
		uniformBufferAligment = uniformBufferAligment_;
		uniformBufferAligmentBits = static_cast<int>(std::floor(std::log2(uniformBufferAligment_)));

		for (int k = 0; k < JSE_VERTEX_CACHE_FRAMES; ++k)
		{
			AllocBufferSet(transientBufferSet[k], transientCacheSize, transientCacheSize, transientUboCacheSize, BU_DYNAMIC);
			transientBufferSet[k].indexMaxSize = transientCacheSize;
			transientBufferSet[k].vertexMaxSize = transientCacheSize;
			transientBufferSet[k].uniformMaxSize = transientUboCacheSize;
		}
		AllocBufferSet(staticBufferSet, staticCacheSize, staticCacheSize, staticUboCacheSize, BU_STATIC);
		staticBufferSet.indexMaxSize = staticCacheSize;
		staticBufferSet.vertexMaxSize = staticCacheSize;
		staticBufferSet.uniformMaxSize = staticUboCacheSize;

		MapBufferSet(transientBufferSet[activeFrame]);
		MapBufferSet(staticBufferSet);
		
		initialized = true;
		Info("VertexCache initialized");
		Info("================================================================");
		Info("     tVert       tIdx       tUbo      sVert       sIdx       sUbo");
		Info("%10d %10d %10d %10d %10d %10d", transientCacheSize, transientCacheSize, transientUboCacheSize, staticCacheSize, staticCacheSize, staticUboCacheSize);
		Info("================================================================");

		return true;
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

	void VertexCache::ClearStaticCache()
	{
		staticBufferSet.indexAlloced.store(0, std::memory_order_relaxed);
		staticBufferSet.vertexAlloced.store(0, std::memory_order_relaxed);
		//staticBufferSet.uniformsAlloced.store(0, std::memory_order_relaxed);
	}

	void VertexCache::PrintStatistic() const
	{
		Info("Allocated static vertex     :   %d", staticBufferSet.vertexAlloced.load());
		Info("Allocated static index      :   %d", staticBufferSet.indexAlloced.load());
		Info("Allocated static uniform    :   %d", staticBufferSet.uniformsAlloced.load());
		Info("Allocated transient vertex  :   %d", transientBufferSet[activeFrame].vertexAlloced.load());
		Info("Allocated transient index   :   %d", transientBufferSet[activeFrame].indexAlloced.load());
		Info("Allocated transient uniform :   %d", transientBufferSet[activeFrame].uniformsAlloced.load());

	}

	vertCacheHandle_t VertexCache::AllocStaticVertex(const void* data, int size)
	{
		return RealAlloc(staticBufferSet, data, size, CACHE_VERTEX);
	}

	vertCacheHandle_t VertexCache::AllocStaticIndex(const void* data, int size)
	{
		return RealAlloc(staticBufferSet, data, size, CACHE_INDEX);
	}

	vertCacheHandle_t VertexCache::AllocStaticUniform(const void* data, int size)
	{
		return RealAlloc(staticBufferSet, data, size, CACHE_UNIFORM);
	}

	vertCacheHandle_t VertexCache::AllocTransientVertex(const void* data, int size)
	{
		return RealAlloc(transientBufferSet[listNum], data, size, CACHE_VERTEX);
	}

	vertCacheHandle_t VertexCache::AllocTransientIndex(const void* data, int size)
	{
		return RealAlloc(transientBufferSet[listNum], data, size, CACHE_INDEX);
	}

	vertCacheHandle_t VertexCache::AllocTransientUniform(const void* data, int size)
	{
		return RealAlloc(transientBufferSet[listNum], data, size, CACHE_UNIFORM);
	}

	bool VertexCache::GetVertexBuffer(vertCacheHandle_t handle, VertexBuffer& dest)
	{
		const bool		isStatic = (handle >> CACHE_STATIC_SHIFT) & 1;
		const int		offset = int((handle >> CACHE_OFFSET_SHIFT) & CACHE_OFFSET_MASK);
		const int		size = int((handle >> CACHE_SIZE_SHIFT) & CACHE_SIZE_MASK);
		const int		frameNum = int((handle >> CACHE_FRAME_SHIFT) & CACHE_FRAME_MASK);

		if (isStatic)
		{
			dest.MakeView(staticBufferSet.vertexBuffer, offset, size);
			return true;
		}
		if (frameNum != ((activeFrame - 1) & CACHE_FRAME_MASK))
		{
			return false;
		}
		dest.MakeView(transientBufferSet[renderFrame].vertexBuffer, offset, size);
	}

	bool VertexCache::GetIndexBuffer(vertCacheHandle_t handle, IndexBuffer& dest)
	{
		const bool		isStatic = (handle >> CACHE_STATIC_SHIFT) & 1;
		const int		offset = int((handle >> CACHE_OFFSET_SHIFT) & CACHE_OFFSET_MASK);
		const int		size = int((handle >> CACHE_SIZE_SHIFT) & CACHE_SIZE_MASK);
		const int		frameNum = int((handle >> CACHE_FRAME_SHIFT) & CACHE_FRAME_MASK);

		if (isStatic)
		{
			dest.MakeView(staticBufferSet.indexBuffer, offset, size);
			return true;
		}
		if (frameNum != ((activeFrame - 1) & CACHE_FRAME_MASK))
		{
			return false;
		}
		dest.MakeView(transientBufferSet[renderFrame].indexBuffer, offset, size);
	}

	bool VertexCache::GetUniformBuffer(vertCacheHandle_t handle, UniformBuffer& dest)
	{
		const bool		isStatic = (handle & CACHE_STATIC);
		const int		offset = int((handle >> CACHE_OFFSET_SHIFT) & CACHE_OFFSET_MASK);
		const int		size = int((handle >> CACHE_SIZE_SHIFT) & CACHE_SIZE_MASK);
		const int		frameNum = int((handle >> CACHE_FRAME_SHIFT) & CACHE_FRAME_MASK);

		if (isStatic)
		{
			dest.MakeView(staticBufferSet.uniformBuffer, offset, size);
			return true;
		}
		if (frameNum != ((activeFrame - 1) & CACHE_FRAME_MASK))
		{
			return false;
		}
		dest.MakeView(transientBufferSet[renderFrame].uniformBuffer, offset, size);
	}

	bool VertexCache::IsStatic(vertCacheHandle_t handle) const
	{
		return handle & CACHE_STATIC;
	}

	bool VertexCache::IsCurrent(vertCacheHandle_t handle) const
	{
		if (handle & CACHE_STATIC)
		{
			return true;
		}
		
		return (activeFrame & CACHE_FRAME_MASK) == (int)((handle >> CACHE_FRAME_SHIFT) & CACHE_FRAME_MASK);
	}

	byte* VertexCache::MappedVertex(vertCacheHandle_t handle) const
	{
		assert(!IsStatic(handle));
		const uint64 offset		= (handle >> CACHE_OFFSET_SHIFT) & CACHE_OFFSET_MASK;
		const uint64 framenum	= (handle >> CACHE_FRAME_SHIFT) & CACHE_FRAME_MASK;
		assert(framenum == (activeFrame & CACHE_FRAME_MASK));

		return transientBufferSet[listNum].vertexPtr + offset;
	}
	byte* VertexCache::MappedIndex(vertCacheHandle_t handle) const
	{
		assert(!IsStatic(handle));
		const uint64 offset = (handle >> CACHE_OFFSET_SHIFT) & CACHE_OFFSET_MASK;
		const uint64 framenum = (handle >> CACHE_FRAME_SHIFT) & CACHE_FRAME_MASK;
		assert(framenum == (activeFrame & CACHE_FRAME_MASK));

		return transientBufferSet[listNum].indexPtr + offset;
	}
	byte* VertexCache::MappedUniform(vertCacheHandle_t handle) const
	{
		assert(!IsStatic(handle));
		const uint64 offset = (handle >> CACHE_OFFSET_SHIFT) & CACHE_OFFSET_MASK;
		const uint64 framenum = (handle >> CACHE_FRAME_SHIFT) & CACHE_FRAME_MASK;
		assert(framenum == (activeFrame & CACHE_FRAME_MASK));

		return transientBufferSet[listNum].uniformsPtr + offset;
	}

	void VertexCache::BindVertexBuffer(vertCacheHandle_t handle, int binding, uint32 stride) const
	{
		VertexBuffer const* buffer;
		const bool isStatic = (handle & CACHE_STATIC) == CACHE_STATIC;
		const uint64 offset = (handle >> CACHE_OFFSET_SHIFT) & CACHE_OFFSET_MASK;
		const uint64 framenum = (handle >> CACHE_FRAME_SHIFT) & CACHE_FRAME_MASK;

		if (isStatic)
		{
			buffer = &staticBufferSet.vertexBuffer;
		}
		else if (framenum == activeFrame - 1)
		{
			buffer = &transientBufferSet[renderFrame].vertexBuffer;
		}
		else return;

		buffer->BindVertexBuffer(binding, offset, stride);
	}

	void VertexCache::BindIndexBuffer(vertCacheHandle_t handle) const
	{
		IndexBuffer const* buffer;
		const bool isStatic = (handle & CACHE_STATIC) == CACHE_STATIC;
		const uint64 offset = (handle >> CACHE_OFFSET_SHIFT) & CACHE_OFFSET_MASK;
		const uint64 framenum = (handle >> CACHE_FRAME_SHIFT) & CACHE_FRAME_MASK;

		if (isStatic)
		{
			buffer = &staticBufferSet.indexBuffer;
		}
		else if (framenum == activeFrame - 1)
		{
			buffer = &transientBufferSet[renderFrame].indexBuffer;
		}
		else return;

		buffer->BindIndexBuffer();
	}

	uint32 VertexCache::GetBaseVertex(vertCacheHandle_t handle, uint32 vertexSize) const
	{
		const uint64 offset = (handle >> CACHE_OFFSET_SHIFT) & CACHE_OFFSET_MASK;

		return (uint32)(offset / vertexSize);
	}

	vertCacheHandle_t VertexCache::RealAlloc(geoBufferSet_t& gbs, const void* data, int bytes, eCacheType type)
	{
		if (bytes == 0)
		{
			return (vertCacheHandle_t)0;
		}

		assert((bytes & 15) == 0);

		int	endPos = 0;
		int offset = 0;
		int numBytes{};
		bool bStatic = false;
		char* sStatic;

		if (&gbs == &staticBufferSet)
		{
			bStatic = true;
			sStatic = "static";
		}
		else
		{
			sStatic = "transient";
		}

		switch (type)
		{
		case CACHE_VERTEX:
			numBytes = (bytes + 31) & ~31;
			endPos = gbs.vertexAlloced.fetch_add(bytes, std::memory_order_relaxed) + numBytes;
			if (endPos > gbs.vertexMaxSize)
			{
				Error("Out of vertex cache !");
			}
			offset = endPos - numBytes;
			if (data != nullptr)
			{
				if (gbs.vertexBuffer.GetUsage() == BU_DYNAMIC)
				{
					MapBufferSet(gbs);
				}
				gbs.vertexBuffer.Update(data, offset, bytes);
			}
			break;

		case CACHE_INDEX:
			numBytes = (bytes + 15) & ~15;
			endPos = gbs.indexAlloced.fetch_add(bytes, std::memory_order_relaxed) + numBytes;
			if (endPos > gbs.indexMaxSize)
			{
				Error("Out of index cache !");
			}
			offset = endPos - numBytes;
			if (data != nullptr)
			{
				if (gbs.indexBuffer.GetUsage() == BU_DYNAMIC)
				{
					MapBufferSet(gbs);
				}
				gbs.indexBuffer.Update(data, offset, bytes);
			}
			break;

		case CACHE_UNIFORM:
			numBytes = (bytes + (uniformBufferAligment - 1)) & ~(uniformBufferAligment - 1);
			endPos = gbs.uniformsAlloced.fetch_add(numBytes, std::memory_order_relaxed) + numBytes;
			if (endPos > gbs.uniformMaxSize)
			{
				Error("Out of uniform cache !");
			}
			offset = endPos - numBytes;
			if (data != nullptr)
			{
				if (gbs.uniformBuffer.GetUsage() == BU_DYNAMIC)
				{
					MapBufferSet(gbs);
				}
				gbs.uniformBuffer.Update(data, offset, bytes);
			}
			break;
		default:
			assert(false);
		}

		vertCacheHandle_t handle =	((uint64)(activeFrame & CACHE_FRAME_MASK) << CACHE_FRAME_SHIFT) |
									((uint64)(offset & CACHE_OFFSET_MASK) << CACHE_OFFSET_SHIFT) |
									((uint64)(bytes & CACHE_SIZE_MASK) << CACHE_SIZE_SHIFT);

		if (&gbs == &staticBufferSet)
		{
			handle |= CACHE_STATIC;
		}

		return handle;
	}

}
