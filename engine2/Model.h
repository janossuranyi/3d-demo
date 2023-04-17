#pragma once
#include "./Bounds.h"
#include "./RenderCommon.h"

namespace jsr {

	struct vertexData_t
	{
		Bounds				bounds;
		int					numVerts;
		drawVert_t*			verts;
		int					numIndexes;
		elementIndex_t*		indexes;
		eTopology			topology;
		vertCacheHandle_t	vertexCache;
		vertCacheHandle_t	indexCache;
	};


	struct modelSurface_t
	{
		int				id;
		const Material* shader;
		vertexData_t	primitive;
	};

	class RenderModel
	{

	};
}