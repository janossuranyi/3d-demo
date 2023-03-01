#include "JSE.h"

namespace js {

	VertexCache::VertexCache(GfxRenderer* hwr) : 
		VertexCache(hwr, DEFAULT_MAX_STATIC_CACHE, DEFAULT_MAX_TRANSIENT_CACHE)
	{
	}

	VertexCache::VertexCache(GfxRenderer* hwr, int maxStaticCache, int maxTransientCache)
	{
		staticCacheSize_ = maxStaticCache;
		transientCacheSize_ = maxTransientCache;
		hwr_ = hwr;
	}

	VertexCache::~VertexCache()
	{
	}

	void VertexCache::Init()
	{
		staticBufferSet_.vertexBuffer = hwr_->CreateBuffer();
		staticBufferSet_.indexBuffer = hwr_->CreateBuffer();
		for (int k = 0; k < 2; ++k) {
			transientBufferSet_[k].vertexBuffer = hwr_->CreateBuffer();
			transientBufferSet_[k].indexBuffer = hwr_->CreateBuffer();

			auto* i = hwr_->CreateCommand<JseCmdCreateBuffer>();
			i->info.bufferId = transientBufferSet_[k].vertexBuffer;
			i->info.size = transientCacheSize_;
			i->info.storageFlags = JSE_BUFFER_STORAGE_DYNAMIC_BIT;
			i->info.target = JseBufferTarget::VERTEX;

			i = hwr_->CreateCommand<JseCmdCreateBuffer>();
			i->info.bufferId = transientBufferSet_[k].indexBuffer;
			i->info.size = transientCacheSize_;
			i->info.storageFlags = JSE_BUFFER_STORAGE_DYNAMIC_BIT;
			i->info.target = JseBufferTarget::INDEX;
		}

		auto* i = hwr_->CreateCommand<JseCmdCreateBuffer>();
		i->info.bufferId = staticBufferSet_.vertexBuffer;
		i->info.size = staticCacheSize_;
		i->info.storageFlags = {};
		i->info.target = JseBufferTarget::VERTEX;

		i = hwr_->CreateCommand<JseCmdCreateBuffer>();
		i->info.bufferId = staticBufferSet_.indexBuffer;
		i->info.size = staticCacheSize_;
		i->info.storageFlags = {};
		i->info.target = JseBufferTarget::INDEX;
	}

	void VertexCache::ShutDown()
	{
		auto* i = hwr_->CreateCommand<JseCmdDeleteBuffer>();
		i->buffer = staticBufferSet_.vertexBuffer;
		i = hwr_->CreateCommand<JseCmdDeleteBuffer>();
		i->buffer = staticBufferSet_.indexBuffer;
		for (int k = 0; k < 2; ++k) 
		{
			i = hwr_->CreateCommand<JseCmdDeleteBuffer>();
			i->buffer = transientBufferSet_[k].vertexBuffer;			
			i = hwr_->CreateCommand<JseCmdDeleteBuffer>();
			i->buffer = transientBufferSet_[k].indexBuffer;
		}
	}

	void VertexCache::Frame()
	{
		std::swap(activeFrame_, renderFrame_);
		transientBufferSet_[activeFrame_].vertexAlloced.store(0, std::memory_order_relaxed);
		transientBufferSet_[activeFrame_].indexAlloced.store(0, std::memory_order_relaxed);
	}

	CacheHandle VertexCache::AllocStaticVertex(int vertexCount, int vertexSize)
	{
		uint64_t r = 1ULL | RealAllocVertex(staticBufferSet_, vertexCount * vertexSize);

		return r;
	}

	CacheHandle VertexCache::AllocStaticIndex(int indexCount, int indexSize)
	{
		uint64_t r = 1ULL | RealAllocIndex(staticBufferSet_, indexCount * indexSize);

		return r;
	}

	CacheHandle VertexCache::AllocTransientVertex(int vertexCount, int vertexSize)
	{
		uint64_t r = RealAllocVertex(transientBufferSet_[activeFrame_], vertexCount * vertexSize);

		return r;
	}

	CacheHandle VertexCache::AllocTransientIndex(int indexCount, int indexSize)
	{
		uint64_t r = RealAllocIndex(transientBufferSet_[activeFrame_], indexCount * indexSize);

		return r;
	}

	BufferRange VertexCache::GetVertexBuffer(CacheHandle handle)
	{
		auto r = BufferRange{};
		const bool isStatic = handle & 1;
		int offset = int((handle >> 1) & (1 << 24 - 1));
		int size = int((handle >> 31) & (1 << 30 - 1));
		geoBufferSet_t& geoset = isStatic ? staticBufferSet_ : transientBufferSet_[renderFrame_];
		r.buffer = geoset.vertexBuffer;
		r.offset = offset;
		r.size = size;

		return r;
	}

	BufferRange VertexCache::GetIndexBuffer(CacheHandle handle)
	{
		auto r = BufferRange{};
		const bool isStatic = handle & 1;
		int offset = int((handle >> 1) & (1 << 24 - 1));
		int size = int((handle >> 31) & (1 << 30 - 1));
		geoBufferSet_t& geoset = isStatic ? staticBufferSet_ : transientBufferSet_[renderFrame_];
		r.buffer = geoset.indexBuffer;
		r.offset = offset;
		r.size = size;

		return r;
	}

	void VertexCache::ResetStaticBufferSet()
	{
		staticBufferSet_.vertexAlloced.store(0);
		staticBufferSet_.indexAlloced.store(0);
	}

	CacheHandle VertexCache::RealAllocVertex(geoBufferSet_t& geoset, int size)
	{
		size = (size + 15) & ~15;
		int offset = geoset.vertexAlloced.fetch_add(size, std::memory_order_relaxed);

		uint64_t r = ((uint64_t)offset << 1) | ((uint64_t)size << 31) ;

		return r;
	}

	CacheHandle VertexCache::RealAllocIndex(geoBufferSet_t& geoset, int size)
	{
		size = (size + 15) & ~15;
		int offset = geoset.indexAlloced.fetch_add(size, std::memory_order_relaxed);

		CacheHandle r = ((uint64_t)offset << 1) | ((uint64_t)size << 31);
		
		return r;
	}

}
