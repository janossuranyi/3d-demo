#pragma once

#include "../common.h"
#include "./gfx.h"
#include "./vertex.h"
#include "logger.h"

namespace gfx {

	using vtxCacheHandle = uint64;

//	template<typename T, int StaticBytes = 65536, int FrameBytes = 65536>
	class VertexCache
	{
	public:

		VertexCache() = default;
		~VertexCache();

		void start(Renderer* renderer, size_t staticBytes, size_t frameBytes);

		struct geoBufferSet
		{
			VertexBufferHandle pb;
			VertexBufferHandle vb;
			IndexBufferHandle ib;
			std::atomic_ulong position_alloced;
			std::atomic_ulong vertex_alloced;
			std::atomic_ulong index_alloced;
			uint vertex_size;
			uint index_size;

			geoBufferSet() :
				vb(),
				ib(),
				vertex_alloced(0),
				index_alloced(0),
				position_alloced(0),
				vertex_size(0),
				index_size(0) {}

			geoBufferSet(uint vSize, uint iSize) : geoBufferSet()
			{
				vertex_size = vSize;
				index_size = iSize;
			}
		};

		void resetStaticBufferSet();
		vtxCacheHandle allocStaticPosition(Memory data);
		vtxCacheHandle allocStaticVertex(Memory data);
		vtxCacheHandle allocStaticIndex(Memory data);
		vtxCacheHandle allocVertex(Memory data);
		vtxCacheHandle allocIndex(Memory data);

		template<typename T>
		VertexBufferHandle getPositionBuffer(const vtxCacheHandle handle, uint& offset, uint& size) const;
		template<typename T>
		VertexBufferHandle getVertexBuffer(const vtxCacheHandle handle, uint& offset, uint& size) const;
		template<typename T>
		IndexBufferHandle getIndexBuffer(const vtxCacheHandle handle, uint& offset, uint& size) const;

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

	template<typename T>
	VertexBufferHandle VertexCache::getPositionBuffer(const vtxCacheHandle handle, uint& offset, uint& size) const
	{
		size = (handle & 0xFFFFFFFE) / sizeof(T);
		offset = (handle >> 31) / sizeof(T);

		return static_buffer_set_.pb;
	}

	template<typename T>
	VertexBufferHandle VertexCache::getVertexBuffer(const vtxCacheHandle handle, uint& offset, uint& size) const
	{
		bool isStatic = handle & 1;
		size = (handle & 0xFFFFFFFE) / sizeof(T);
		offset = (handle >> 31) / sizeof(T);

		if (isStatic) return static_buffer_set_.vb;
		else return render_->vb;
	}

	template<typename T>
	IndexBufferHandle VertexCache::getIndexBuffer(const vtxCacheHandle handle, uint& byte_offset, uint& size) const
	{
		bool isStatic = handle & 1;
		size = (handle & 0xFFFFFFFE) / sizeof(T);
		byte_offset = (handle >> 31);

		if (isStatic) return static_buffer_set_.ib;
		else return render_->ib;
	}

}