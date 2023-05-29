#pragma once

#include <vector>
#include "./Bounds.h"
#include "./RenderCommon.h"
#include "./Heap.h"

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

		~surface_t()
		{
			if (verts) { MemFree(verts); }
			if (indexes) { MemFree(indexes); }
		}
		surface_t() :
			bounds(),
			numVerts(),
			verts(),
			numIndexes(),
			indexes(),
			topology(TP_TRIANGLES),
			vertexCache(),
			indexCache(),
			gpuResident(false) {}
	};


	struct modelSurface_t
	{
		int				id;
		const Material* shader;
		surface_t		surf;
		~modelSurface_t();
	};

	class ModelManager;
	class RenderModel
	{
		friend class ModelManager;
	public:
		RenderModel();
		~RenderModel();
		int AllocSurface(int numVerts, int numIndexes);
		void FreeGeometry();
		void UpdateSurfaceCache();
		void SetStatic(bool b);
		void MakeUnitRect();
		void MakeUnitCube();
		void SetName(const std::string& name);
		std::string GetName() const;
		inline bool IsStatic() const { return isStatic; }
		inline int GetNumSurface() const { return surfs.size(); }
		inline Bounds GetBounds() const { return bounds; }
		inline Bounds& GetBounds() { return bounds; }
		inline int GetId() const { return id; }
		inline modelSurface_t* GetSurface(int idx)
		{
			assert(idx < surfs.size());
			return &surfs[idx];
		}

	private:
		int id;
		Bounds bounds;
		bool isStatic;
		std::string name;
		std::vector<modelSurface_t> surfs;
	};


	class ModelManager
	{
	public:
		ModelManager();
		~ModelManager();
		RenderModel* CreateModel(const std::string& name);
		RenderModel* FindByName(const std::string& name);
		void RemoveModel(RenderModel* model);
	private:
		std::vector<RenderModel*> models;
		std::vector<int> freelist;
	};
}