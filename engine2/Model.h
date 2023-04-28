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
		RenderModel();
		~RenderModel();
		modelSurface_t*					AllocSurface(int numVerts, int numIndexes, int& newIdx);

		inline int						GetNumSurface() const { return surfs.size(); }
		inline Bounds					GetBounds() const { return bounds; }
		inline modelSurface_t const*	GetSurface(int idx) const
		{
			assert(idx < surfs.size());
			return surfs[idx];
		}

		bool							LoadFromGLTF(const std::string& filename, int index = -1, const std::string name = "");
	private:
		Bounds bounds;
		std::vector<modelSurface_t*> surfs;
	};


	class ModelManager
	{
	public:
		ModelManager();
		~ModelManager();
	private:
		std::vector<RenderModel*> models;
	};
}