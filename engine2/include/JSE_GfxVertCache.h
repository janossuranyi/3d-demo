#ifndef JSE_VERT_CACHE_H
#define JSE_VERT_CACHE_H

#define DEFAULT_MAX_STATIC_CACHE	(128*1024*1024)
#define DEFAULT_MAX_TRANSIENT_CACHE (16*1024*1024)

#define JSE_VERTEX_CACHE_STATIC_BIT (1)
#define JSE_VERTEX_CACHE_OFFSET_BITS  (30)
#define JSE_VERTEX_CACHE_SIZE_BITS  (24)

namespace js {
	using CacheHandle = uint64_t;

	class GfxRenderer;
	class VertexCache {
	public:
		VertexCache();
		VertexCache(int maxStaticCache, int maxTransientCache);
		~VertexCache();
		void Init(GfxRenderer* hwr);
		void ShutDown(GfxRenderer* hwr);
		void Frame();
		CacheHandle AllocStaticVertex(int vertexCount, int vertexSize = sizeof(GfxDrawVert));
		CacheHandle AllocStaticIndex(int indexCount, int indexSize = 2);
		CacheHandle AllocTransientVertex(int vertexCount, int vertexSize = sizeof(GfxDrawVert));
		CacheHandle AllocTransientIndex(int indexCount, int indexSize = 2);
		BufferRange GetVertexBuffer(CacheHandle handle);
		BufferRange GetIndexBuffer(CacheHandle handle);
		void ResetStaticBufferSet();

	private:
		struct geoBufferSet_t {
			JseBufferID			vertexBuffer;
			JseBufferID			indexBuffer;
			std::atomic_int		vertexAlloced;
			std::atomic_int		indexAlloced;
		};

		int staticCacheSize_{};
		int transientCacheSize_{};
		geoBufferSet_t staticBufferSet_{};
		geoBufferSet_t transientBufferSet_[2]{};
		int activeFrame_{ 0 };
		int renderFrame_{ 1 };
		CacheHandle RealAllocVertex(geoBufferSet_t& geoset, int size);
		CacheHandle RealAllocIndex(geoBufferSet_t& geoset, int size);
	};
}
#endif // !JSE_VERT_CACHE_H
