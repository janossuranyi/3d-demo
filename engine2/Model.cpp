#include <filesystem>
#include <utility>
#include <cinttypes>
#include <tiny_gltf.h>

#include "./Math.h"
#include "./Model.h"
#include "./Logger.h"
#include "./GpuTypes.h"
#include "./RenderSystem.h"
#include "./Heap.h"
#include "./System.h"

#define ACCESSOR_PTR(model, accessor) \
(unsigned char*)(model.buffers[model.bufferViews[accessor.bufferView].buffer].data.data() + \
model.bufferViews[accessor.bufferView].byteOffset + \
accessor.byteOffset)

namespace jsr {

	using namespace tinygltf;
	namespace fs = std::filesystem;

	RenderModel::RenderModel() : isStatic(true), id(-1)
	{
	}

	RenderModel::~RenderModel()
	{
		FreeGeometry();
	}

	int RenderModel::AllocSurface(int numVerts, int numIndexes)
	{
		assert(numVerts > 0);
		surfs.emplace_back();
		int newIdx = surfs.size() - 1;
		modelSurface_t& surf = surfs[newIdx];

		surf.surf.numVerts = numVerts;
		surf.surf.numIndexes = numIndexes;
		surf.surf.verts = (drawVert_t*) MemAlloc(numVerts * sizeof(drawVert_t));
		if (numIndexes > 0)
		{
			surf.surf.indexes = (elementIndex_t*) MemAlloc(sizeof(elementIndex_t) * numIndexes);
		}
		surf.id = jsr::GetTimeMillisecond() + GetUniqId();

		return newIdx;
	}

	void RenderModel::FreeGeometry()
	{
		surfs.clear();
		bounds = Bounds{};
	}
	
	ModelManager::ModelManager() {}
	ModelManager::~ModelManager() 
	{
		for (auto* model : models)
		{
			if (model) delete model;
		}
	}

	RenderModel* ModelManager::CreateModel(const std::string& name)
	{
		if (freelist.empty() == false)
		{
			int idx = freelist.back();
			freelist.pop_back();
			models[idx] = new RenderModel();
			models[idx]->SetStatic(true);
			models[idx]->id = idx;
			models[idx]->SetName(name);
			return models[idx];
		}
		models.push_back(new RenderModel());
		RenderModel* model = models.back();
		model->id = models.size() - 1;
		model->SetName(name);
		model->SetStatic(true);

		return model;
	}

	RenderModel* ModelManager::FindByName(const std::string& name)
	{
		for (auto* m : models)
		{
			if (m->GetName() == name) return m;
		}
		return nullptr;
	}

	void ModelManager::RemoveModel(RenderModel* model)
	{
		if (!model) return;
		int id = model->GetId();
		freelist.push_back(id);
		models[id] = nullptr;

		delete model;
	}

	void RenderModel::UpdateSurfaceCache()
	{
		if (isStatic)
		{
			for (auto& surf : surfs)
			{
				if (surf.surf.vertexCache == 0) {
					surf.surf.vertexCache = renderSystem.vertexCache->AllocStaticVertex(surf.surf.verts, sizeof(drawVert_t) * surf.surf.numVerts);
				}
				if (surf.surf.indexCache == 0) {
					surf.surf.indexCache = renderSystem.vertexCache->AllocStaticIndex(surf.surf.indexes, (15UL + sizeof(elementIndex_t) * surf.surf.numIndexes) & ~15UL);
				}
				surf.surf.gpuResident = true;
				MemFree16(surf.surf.verts); surf.surf.verts = nullptr;
				MemFree16(surf.surf.indexes); surf.surf.indexes = nullptr;

				/*
				MemFree(surf->surf.indexes);
				MemFree(surf->surf.verts);
				delete surf;
				*/
			}
			//surfs.clear();
		}
		else
		{
			for (auto& surf : surfs)
			{
				surf.surf.vertexCache = renderSystem.vertexCache->AllocTransientVertex(surf.surf.verts, sizeof(drawVert_t) * surf.surf.numVerts);
				surf.surf.indexCache = renderSystem.vertexCache->AllocTransientIndex(surf.surf.indexes, (15UL + sizeof(elementIndex_t) * surf.surf.numIndexes) & ~15UL) ;
			}
		}
	}
	void RenderModel::SetStatic(bool b)
	{
		isStatic = b;
	}
	void RenderModel::MakeUnitRect()
	{
		static const glm::vec3 verts[] = {
			{-1.0f, -1.0f, 0.0f},
			{-1.0f, 1.0f, 0.0f},
			{1.0f, 1.0f, 0.0f},
			{1.0f, 1.0f, 0.0f}
		};

		if (!surfs.empty())
		{
			FreeGeometry();
		}
		int surfId = AllocSurface(4, 6);
		auto* rectSurf = GetSurface(surfId);
		for (int i = 0; i < 4; ++i)
		{
			rectSurf->surf.verts[i].SetPos(verts[i]);
			rectSurf->surf.verts[i].SetUV(glm::vec2((verts[i] + 1.0f) * 0.5f));
			rectSurf->surf.verts[i].SetNormal(0.0f, 0.0f, 1.0f);
			rectSurf->surf.verts[i].SetTangent(1.0f, 0.0f, 0.0f, 1.0f);
			rectSurf->surf.verts[i].SetColor(1.0f, 1.0f, 1.0f, 1.0f);
			bounds << rectSurf->surf.verts[i].GetPos();
		}
		const elementIndex_t indices[] = { 0,1,2,2,3,0 };
		memcpy(rectSurf->surf.indexes, indices, sizeof(indices));
		if (isStatic)
		{
			UpdateSurfaceCache();
		}
	}
	void RenderModel::MakeUnitCube()
	{
	}
	void RenderModel::SetName(const std::string& name)
	{
		this->name = name;
	}
	std::string RenderModel::GetName() const
	{
		return name;
	}
	modelSurface_t::~modelSurface_t()
	{
		//if (shader) delete shader;
	}
}
