#ifndef JSR_VERTEX_CACHE_H
#define JSR_VERTEX_CACHE_H

#define JSE_VERTEX_CACHE_FRAMES			3

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
		bool Init(int uniformBufferAligment_ = 256);
		void Shutdown();
		void Restart();
		void Frame();
		void ClearStaticCache();
		void PrintStatistic() const;
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
		void BindVertexBuffer(vertCacheHandle_t handle, int binding, uint32 stride) const;
		void BindIndexBuffer(vertCacheHandle_t handle) const;
		uint32 GetBaseVertex(vertCacheHandle_t handle, uint32 vertexSize) const;
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
