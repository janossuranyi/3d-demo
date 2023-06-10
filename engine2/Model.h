#pragma once

#include <vector>
#include <tiny_gltf.h>
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
		void SetName(const std::string& name);
		void CreateFromTris(surface_t* tris);
		std::string GetName() const;
		inline bool IsStatic() const { return isStatic; }
		inline int GetNumSurface() const { return surfs.size(); }
		inline Bounds GetBounds() const { return bounds; }
		inline Bounds& GetBounds() { return bounds; }
		inline int GetId() const { return id; }
		inline modelSurface_t* GetSurface(int idx)
		{
			assert(idx < surfs.size());
			return surfs[idx];
		}

	private:
		int id;
		Bounds bounds;
		bool isStatic;
		std::string name;
		std::vector<modelSurface_t*> surfs;
	};

	template<class T>
	struct AccessorArray
	{
		AccessorArray(const tinygltf::Model& model, const tinygltf::Accessor& accessor)
		{

			const auto& view = model.bufferViews[accessor.bufferView];
			const auto& buff = model.buffers[view.buffer];

			size_t offset1 = accessor.byteOffset;
			size_t offset2 = view.byteOffset;
			size = view.byteLength;

			stride = accessor.ByteStride(view);

			ptr = (const char*)(buff.data.data() + offset1 + offset2);
		}

		const T* operator[](size_t index) const
		{
			size_t offset = index * stride;
			assert(offset < size);
			return (const T*)(ptr + offset);
		}

		const char* ptr;
		size_t stride;
		size_t size;
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