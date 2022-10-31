#pragma once

#include <atomic>
#include "gfx.h"
#include "vertex.h"

namespace gfx {

	using vtxCacheHandle = uint64_t;

	template<typename T, int StaticBytes = 65536, int FrameBytes = 65536>
	class VertexCache
	{
	public:

		static const uint32_t static_bytes = StaticBytes;
		static const uint32_t frame_bytes = FrameBytes;
		using base_type = typename T;

		VertexCache() = default;
		~VertexCache();

		void start(Renderer* renderer);

		struct geoBufferSet
		{
			VertexBufferHandle vb;
			IndexBufferHandle ib;
			std::atomic_ulong vertex_alloced;
			std::atomic_ulong index_alloced;
			uint32_t vertex_size;
			uint32_t index_size;

			geoBufferSet() :
				vb(),
				ib(),
				vertex_alloced(0),
				index_alloced(0),
				vertex_size(0),
				index_size(0) {}

			geoBufferSet(uint32_t vSize, uint32_t iSize) : geoBufferSet()
			{
				vertex_size = vSize;
				index_size = iSize;
			}
		};

		vtxCacheHandle allocStaticVertex(Memory data);
		vtxCacheHandle allocStaticIndex(Memory data);
		vtxCacheHandle allocVertex(Memory data);
		vtxCacheHandle allocIndex(Memory data);

		VertexBufferHandle getVertexBuffer(vtxCacheHandle handle, uint32_t& offset, uint32_t& size);
		IndexBufferHandle getIndexBuffer(vtxCacheHandle handle, uint32_t& offset, uint32_t& size);

		void frame();
	private:
		vtxCacheHandle realVertexAlloc(Memory data, geoBufferSet& bs);
		vtxCacheHandle realIndexAlloc(Memory data, geoBufferSet& bs);

		Renderer* renderer_;
		geoBufferSet static_buffer_set_;
		geoBufferSet dynamic_buffer_set_[2];
		geoBufferSet* submit_;
		geoBufferSet* render_;
	};

/*****************************************************/
/**               IMPLEMENTATION                     */
/*****************************************************/

	template<typename T, int StaticBytes, int FrameBytes>
	VertexBufferHandle VertexCache<T, StaticBytes, FrameBytes>::getVertexBuffer(vtxCacheHandle handle, uint32_t& offset, uint32_t& size)
	{
		bool isStatic = handle & 1;
		size = (handle & 0xFFFFFFFE) / sizeof(T);
		offset = (handle >> 31) / sizeof(T);

		if (isStatic) return static_buffer_set_.vb;
		else return render_->vb;
	}

	template<typename T, int StaticBytes, int FrameBytes>
	IndexBufferHandle VertexCache<T, StaticBytes, FrameBytes>::getIndexBuffer(vtxCacheHandle handle, uint32_t& offset, uint32_t& size)
	{
		bool isStatic = handle & 1;
		size = (handle & 0xFFFFFFFE) / sizeof(T);
		offset = (handle >> 31) / sizeof(T);

		if (isStatic) return static_buffer_set_.ib;
		else return render_->ib;
	}


	template<typename T, int StaticBytes, int FrameBytes>
	VertexCache<T, StaticBytes, FrameBytes>::~VertexCache()
	{
		renderer_->deleteVertexBuffer(static_buffer_set_.vb);
		renderer_->deleteIndexBuffer(static_buffer_set_.ib);
		for (int i = 0; i < 2; ++i)
		{
			renderer_->deleteVertexBuffer(dynamic_buffer_set_[i].vb);
			renderer_->deleteIndexBuffer(dynamic_buffer_set_[i].ib);
		}
	}

	template<typename T, int StaticBytes, int FrameBytes>
	void VertexCache<T, StaticBytes, FrameBytes>::start(Renderer* r)
	{
		renderer_ = r;
		static_buffer_set_.index_size = StaticBytes;
		static_buffer_set_.vertex_size = StaticBytes;
		static_buffer_set_.vb = renderer_->createVertexBuffer(StaticBytes, BufferUsage::Static, Memory());
		static_buffer_set_.ib = renderer_->createIndexBuffer(StaticBytes, BufferUsage::Static, Memory());
		static_buffer_set_.index_alloced = 0;
		static_buffer_set_.vertex_alloced = 0;

		for (int i = 0; i < 2; ++i)
		{
			dynamic_buffer_set_[i].index_size = FrameBytes;
			dynamic_buffer_set_[i].vertex_size = FrameBytes;
			dynamic_buffer_set_[i].vb = renderer_->createVertexBuffer(FrameBytes, BufferUsage::Dynamic, Memory());
			dynamic_buffer_set_[i].ib = renderer_->createIndexBuffer(FrameBytes, BufferUsage::Dynamic, Memory());
			dynamic_buffer_set_[i].index_alloced = 0;
			dynamic_buffer_set_[i].vertex_alloced = 0;
		}

		submit_ = &dynamic_buffer_set_[0];
		render_ = &dynamic_buffer_set_[1];
	}


	template<typename T, int StaticBytes, int FrameBytes>
	vtxCacheHandle VertexCache<T, StaticBytes, FrameBytes>::allocStaticVertex(Memory data)
	{
		const auto h = realVertexAlloc(data, static_buffer_set_);
		return h | 1ULL;
	}

	template<typename T, int StaticBytes, int FrameBytes>
	vtxCacheHandle VertexCache<T, StaticBytes, FrameBytes>::allocStaticIndex(Memory data)
	{
		const auto h = realIndexAlloc(data, static_buffer_set_);
		return h | 1ULL;
	}

	template<typename T, int StaticBytes, int FrameBytes>
	vtxCacheHandle VertexCache<T, StaticBytes, FrameBytes>::allocVertex(Memory data)
	{
		return realVertexAlloc(data, *submit_);
	}

	template<typename T, int StaticBytes, int FrameBytes>
	vtxCacheHandle VertexCache<T, StaticBytes, FrameBytes>::allocIndex(Memory data)
	{
		return realIndexAlloc(data, *submit_);
	}

	template<typename T, int StaticBytes, int FrameBytes>
	void VertexCache<T, StaticBytes, FrameBytes>::frame()
	{
		std::swap(submit_, render_);
		submit_->index_alloced.store(0, std::memory_order_relaxed);
		submit_->vertex_alloced.store(0, std::memory_order_relaxed);
	}

	template<typename T, int StaticBytes, int FrameBytes>
	vtxCacheHandle VertexCache<T, StaticBytes, FrameBytes>::realVertexAlloc(Memory data, geoBufferSet& bs)
	{
		const size_t bytes = (data.size() + 15) & (~15);
		if (bytes + bs.vertex_alloced > bs.vertex_size)
		{
			Warning("VertexAlloc failed, not enough free space");
			return 0ULL;
		}
		const size_t offs = bs.vertex_alloced.fetch_add(bytes, std::memory_order_relaxed);
		renderer_->updateVertexBuffer(bs.vb, data, offs);

		return (offs << 31) | bytes;
	}

	template<typename T, int StaticBytes, int FrameBytes>
	vtxCacheHandle VertexCache<T, StaticBytes, FrameBytes>::realIndexAlloc(Memory data, geoBufferSet& bs)
	{
		const size_t bytes = (data.size() + 15) & (~15);
		if (bytes + bs.index_alloced > bs.index_size)
		{
			Warning("IndexAlloc failed, not enough free space");
			return 0ULL;
		}
		const size_t offs = bs.index_alloced.fetch_add(bytes, std::memory_order_relaxed);
		renderer_->updateIndexBuffer(bs.ib, data, offs);

		return (offs << 31) | bytes;
	}

}