#pragma once

#include <atomic>
#include "gfx.h"
#include "vertex.h"

namespace gfx {

	using vtxCacheHandle = uint64_t;

	class VertexCache
	{
	public:

		VertexCache() = default;
		~VertexCache();

		void init(Renderer* renderer, uint32_t staticSize, uint32_t dynSize);

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

		template<typename T>
		VertexBufferHandle getVertexBuffer(vtxCacheHandle handle, uint32_t& offset, uint32_t& size);
		template<typename T>
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

	template<typename T>
	VertexBufferHandle VertexCache::getVertexBuffer(vtxCacheHandle handle, uint32_t& offset, uint32_t& size)
	{
		bool isStatic = handle & 1;
		size = (handle & 0xFFFFFFFE) / sizeof(T);
		offset = (handle >> 31) / sizeof(T);

		if (isStatic) return static_buffer_set_.vb;
		else return render_->vb;
	}

	template<typename T>
	IndexBufferHandle VertexCache::getIndexBuffer(vtxCacheHandle handle, uint32_t& offset, uint32_t& size)
	{
		bool isStatic = handle & 1;
		size = (handle & 0xFFFFFFFE) / sizeof(T);
		offset = (handle >> 31) / sizeof(T);

		if (isStatic) return static_buffer_set_.ib;
		else return render_->ib;
	}


}