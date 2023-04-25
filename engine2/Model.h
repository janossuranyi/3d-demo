#pragma once

#include <vector>
#include "./Bounds.h"
#include "./RenderCommon.h"

namespace jsr {

	struct surface_t
	{
		Bounds				bounds;
		int					numVerts;
		drawVert_t*			verts;
		int					numIndexes;
		elementIndex_t*		indexes;
		eTopology			topology;
		vertCacheHandle_t	vertexCache;
		vertCacheHandle_t	indexCache;
		bool gpuResident;
	};


	struct modelSurface_t
	{
		int				id;
		const Material* shader;
		surface_t		surf;
	};

	class RenderModel
	{
	public:
		int						GetNumSurface() const
		{
			return surfs.size();
		}
		modelSurface_t const*	GetSurface(int idx) const
		{
			assert(idx < surfs.size());
			return surfs[idx];
		}
	private:
		std::vector<modelSurface_t*> surfs;
	};
}