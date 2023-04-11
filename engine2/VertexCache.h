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


#include "./EngineTypes.h"
#include "./DrawVert.h"
#include "./BufferObject.h"
#include <atomic>

namespace jsr {
	using vertCacheHandle_t = uint64;

	struct geoBufferSet_t {
		VertexBuffer		vertexBuffer;
		IndexBuffer			indexBuffer;
		UniformBuffer		uniformBuffer;
		std::atomic_int		vertexAlloced;
		std::atomic_int		indexAlloced;
		std::atomic_int		uniformsAlloced;
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
		vertCacheHandle_t AllocStaticVertex(int vertexCount, int vertexSize = sizeof(drawVert_t));
		vertCacheHandle_t AllocStaticIndex(int indexCount, int indexSize = 2);
		vertCacheHandle_t AllocTransientVertex(int vertexCount, int vertexSize = sizeof(drawVert_t));
		vertCacheHandle_t AllocTransientIndex(int indexCount, int indexSize = 2);
		vertCacheHandle_t AllocStaticUniform(int size);
		vertCacheHandle_t AllocTransientUniform(int size);
		bool GetVertexBuffer(vertCacheHandle_t handle, VertexBuffer& vertexBuffer);
		bool GetIndexBuffer(vertCacheHandle_t handle, IndexBuffer& indexBuffer);
		bool GetUniformBuffer(vertCacheHandle_t handle, UniformBuffer& indexBuffer);
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
		vertCacheHandle_t RealAllocVertex(geoBufferSet_t& geoset, int size);
		vertCacheHandle_t RealAllocIndex(geoBufferSet_t& geoset, int size);
		vertCacheHandle_t RealAllocUniform(geoBufferSet_t& geoset, int size);
	};
}
#endif // !JSE_VERT_CACHE_H
