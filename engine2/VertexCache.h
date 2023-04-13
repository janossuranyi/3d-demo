#ifndef JSR_VERTEX_CACHE_H
#define JSR_VERTEX_CACHE_H

#define DEFAULT_MAX_STATIC_CACHE	(128*1024*1024)
#define DEFAULT_MAX_TRANSIENT_CACHE (16*1024*1024)

#define JSE_VERTEX_CACHE_OFFSET_BITS	20
#define JSE_VERTEX_CACHE_SIZE_BITS		21
#define JSE_VERTEX_CACHE_FRAME_BITS		10

#define JSE_VERTEX_CACHE_FRAMES			3
#define JSE_VERTEX_CACHE_STATIC_SHIFT	0
#define JSE_VERTEX_CACHE_OFFSET_SHIFT	1
#define JSE_VERTEX_CACHE_SIZE_SHIFT		21
#define JSE_VERTEX_CACHE_FRAME_SHIFT	42

#define JSE_VERTEX_CACHE_OFFSET_MASK	((1 << JSE_VERTEX_CACHE_OFFSET_BITS) - 1)
#define JSE_VERTEX_CACHE_SIZE_MASK		((1 << JSE_VERTEX_CACHE_SIZE_BITS) - 1)
#define JSE_VERTEX_CACHE_FRAME_MASK		((1 << JSE_VERTEX_CACHE_FRAME_BITS) - 1)
#define VERTEX_CACHE_STATIC				(1)

#include "./EngineTypes.h"
#include "./DrawVert.h"
#include "./BufferObject.h"
#include <atomic>

namespace jsr {
	using vertCacheHandle_t = uint64;

	enum eCacheType
	{
		CACHE_VERTEX,
		CACHE_INDEX,
		CACHE_UNIFORM
	};

	struct geoBufferSet_t
	{
		VertexBuffer		vertexBuffer;
		IndexBuffer			indexBuffer;
		UniformBuffer		uniformBuffer;
		std::atomic_int		vertexAlloced;
		std::atomic_int		indexAlloced;
		std::atomic_int		uniformsAlloced;
		int					vertexMaxSize;
		int					indexMaxSize;
		int					uniformMaxSize;
		byte* vertexPtr;
		byte* indexPtr;
		byte* uniformsPtr;
	};

	class VertexCache {
	public:

		VertexCache();
		VertexCache(const VertexCache&) = delete;
		VertexCache(VertexCache&&) = delete;
		VertexCache& operator=(const VertexCache&) = delete;
		VertexCache& operator=(VertexCache&&) = delete;
		~VertexCache();
		void Init(int uniformBufferAligment_ = 256);
		void Shutdown();
		void Restart();
		void Frame();
		void ClearStaticCache();
		vertCacheHandle_t AllocStaticVertex(const void* data, int bytes);
		vertCacheHandle_t AllocStaticIndex(const void* data, int bytes);
		vertCacheHandle_t AllocStaticUniform(const void* data, int bytes);
		vertCacheHandle_t AllocTransientVertex(const void* data, int bytes);
		vertCacheHandle_t AllocTransientIndex(const void* data, int bytes);
		vertCacheHandle_t AllocTransientUniform(const void* data, int bytes);
		bool GetVertexBuffer(vertCacheHandle_t handle, VertexBuffer& vertexBuffer);
		bool GetIndexBuffer(vertCacheHandle_t handle, IndexBuffer& indexBuffer);
		bool GetUniformBuffer(vertCacheHandle_t handle, UniformBuffer& indexBuffer);
		bool IsStatic(vertCacheHandle_t handle) const;
		bool IsCurrent(vertCacheHandle_t handle) const;
		byte* MappedVertex(vertCacheHandle_t handle) const;
		byte* MappedIndex(vertCacheHandle_t handle) const;
		byte* MappedUniform(vertCacheHandle_t handle) const;
	private:

		int staticCacheSize{};
		int staticUboCacheSize{};
		int transientCacheSize{};
		int transientUboCacheSize{};
		int activeFrame{ 0 };
		int renderFrame{ 1 };
		int listNum{ 0 };
		int uniformBufferAligment{};
		int uniformBufferAligmentBits{};
		bool initialized{};
		geoBufferSet_t staticBufferSet{};
		geoBufferSet_t transientBufferSet[JSE_VERTEX_CACHE_FRAMES]{};
		vertCacheHandle_t RealAlloc(geoBufferSet_t& gbs, const void* data, int size, eCacheType type);
	};
}
#endif // !JSE_VERT_CACHE_H
