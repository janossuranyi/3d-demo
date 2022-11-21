#include "gfx/vertex_cache.h"

namespace gfx {
	VertexCache::~VertexCache()
	{
		renderer_->deleteVertexBuffer(static_buffer_set_.vb);
		renderer_->deleteIndexBuffer(static_buffer_set_.ib);
		for (int i = 0; i < 2; ++i)
		{
			renderer_->deleteVertexBuffer(dynamic_buffer_set_[i].vb);
			renderer_->deleteIndexBuffer(dynamic_buffer_set_[i].ib);
		}
	}

	void VertexCache::start(Renderer* r, size_t staticBytes, size_t frameBytes)
	{
		renderer_ = r;
		static_buffer_set_.index_size = staticBytes;
		static_buffer_set_.vertex_size = staticBytes;
		static_buffer_set_.pb = renderer_->createVertexBuffer(staticBytes, BufferUsage::Static, Memory());
		static_buffer_set_.vb = renderer_->createVertexBuffer(staticBytes, BufferUsage::Static, Memory());
		static_buffer_set_.ib = renderer_->createIndexBuffer(staticBytes, BufferUsage::Static, Memory());
		static_buffer_set_.index_alloced = 0;
		static_buffer_set_.vertex_alloced = 0;
		static_buffer_set_.position_alloced = 0;

		for (int i = 0; i < 2; ++i)
		{
			dynamic_buffer_set_[i].index_size = frameBytes;
			dynamic_buffer_set_[i].vertex_size = frameBytes;
			dynamic_buffer_set_[i].vb = renderer_->createVertexBuffer(frameBytes, BufferUsage::Dynamic, Memory());
			dynamic_buffer_set_[i].ib = renderer_->createIndexBuffer(frameBytes, BufferUsage::Dynamic, Memory());
			dynamic_buffer_set_[i].index_alloced = 0;
			dynamic_buffer_set_[i].vertex_alloced = 0;
		}

		submit_ = &dynamic_buffer_set_[0];
		render_ = &dynamic_buffer_set_[1];
	}


	void VertexCache::resetStaticBufferSet()
	{
		static_buffer_set_.position_alloced.store(0, std::memory_order_relaxed);
		static_buffer_set_.index_alloced.store(0, std::memory_order_relaxed);
		static_buffer_set_.vertex_alloced.store(0, std::memory_order_relaxed);
	}

	vtxCacheHandle VertexCache::allocStaticPosition(Memory& data)
	{
		const size_t bytes = data.size();

		assert((bytes & 15) == 0);

		if (bytes + static_buffer_set_.position_alloced > static_buffer_set_.vertex_size)
		{
			Warning("PositionAlloc failed, not enough free space");
			return 0ULL;
		}
		const size_t offs = static_buffer_set_.position_alloced.fetch_add(bytes, std::memory_order_relaxed);
		renderer_->updateVertexBuffer(static_buffer_set_.pb, std::move(data), offs);

		return (offs << 31) | (bytes & 0xFFFFFFFF);
	}

	vtxCacheHandle VertexCache::allocStaticVertex(Memory& data)
	{
		const auto h = realVertexAlloc(data, static_buffer_set_);
		return h | 1ULL;
	}

	vtxCacheHandle VertexCache::allocStaticIndex(Memory& data)
	{
		const auto h = realIndexAlloc(data, static_buffer_set_);
		return h | 1ULL;
	}

	vtxCacheHandle VertexCache::allocVertex(Memory& data)
	{
		return realVertexAlloc(data, *submit_);
	}

	vtxCacheHandle VertexCache::allocIndex(Memory& data)
	{
		return realIndexAlloc(data, *submit_);
	}

	void VertexCache::frame()
	{
		std::swap(submit_, render_);
		submit_->index_alloced.store(0, std::memory_order_relaxed);
		submit_->vertex_alloced.store(0, std::memory_order_relaxed);
	}

	vtxCacheHandle VertexCache::realVertexAlloc(Memory& data, geoBufferSet& bs)
	{
		const size_t bytes = data.size();

		assert((bytes & 15) == 0);

		if (bytes + bs.vertex_alloced > bs.vertex_size)
		{
			Warning("VertexAlloc failed, not enough free space");
			return 0ULL;
		}
		const size_t offs = bs.vertex_alloced.fetch_add(bytes, std::memory_order_relaxed);
		renderer_->updateVertexBuffer(bs.vb, std::move(data), offs);

		return (offs << 31) | (bytes & 0xFFFFFFFF);
	}

	vtxCacheHandle VertexCache::realIndexAlloc(Memory& data, geoBufferSet& bs)
	{
		const size_t bytes = data.size();

		assert((bytes & 15) == 0);

		if (bytes + bs.index_alloced > bs.index_size)
		{
			Warning("IndexAlloc failed, not enough free space");
			return 0ULL;
		}
		const size_t offs = bs.index_alloced.fetch_add(bytes, std::memory_order_relaxed);
		renderer_->updateIndexBuffer(bs.ib, std::move(data), offs);

		return (offs << 31) | (bytes & 0xFFFFFFFF);
	}
}