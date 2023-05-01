#include <filesystem>
#include <utility>
#include <cinttypes>
#include <tiny_gltf.h>

#include "./Model.h"
#include "./Logger.h"

namespace jsr {

	using namespace tinygltf;
	namespace fs = std::filesystem;

	RenderModel::RenderModel()
	{
	}

	RenderModel::~RenderModel()
	{
		for (auto* surf : surfs)
		{
			if (surf->surf.verts) { delete surf->surf.verts; }
			if (surf->surf.indexes) { delete surf->surf.indexes; }
			delete surf;
		}
	}
	modelSurface_t* RenderModel::AllocSurface(int numVerts, int numIndexes, int& newIdx)
	{
		assert(numVerts > 0);
		surfs.push_back(new modelSurface_t());
		modelSurface_t* surf = surfs.back();
		newIdx = surfs.size() - 1;

		surf->surf.numVerts = numVerts;
		surf->surf.numIndexes = numIndexes;
		surf->surf.verts = new drawVert_t[numVerts];
		if (numIndexes > 0)
		{
			surf->surf.indexes = new elementIndex_t[numIndexes];
		}

		return surf;
	}
	bool RenderModel::LoadFromGLTF(const std::string& filename, int index, const std::string& name)
	{
		Model model{};
		TinyGLTF loader{};
		std::string err, warn;

		if (name.empty() && index < 0) return false;

		bool wasOk{};
		fs::path fnp(filename);
		if (fnp.extension().string() == ".glb")
		{
			wasOk = loader.LoadBinaryFromFile(&model, &err, &warn, filename);
		}
		else
		{
			wasOk = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
		}

		if (!wasOk)
		{
			Error("[ModelManager]: (LoadFromGLTF) %s", err.c_str());
			return false;
		}

		if (!warn.empty())
		{
			Info("[ModelManager]: (LoadFromGLTF) %s", warn.c_str());
		}

		Mesh const* mesh{};
		if (!name.empty())
		{
			for (auto const& m : model.meshes)
			{
				if (m.name == name)
				{
					mesh = &m;
					break;
				}
			}
		}
		else if (index < model.meshes.size())
		{
			mesh = &model.meshes.at(index);
		}

		if (!mesh)
		{
			Error("[ModelManager]: (LoadFromGLTF) mesh '%s' not found", name.c_str());
			return false;
		}

		int const numSurfs = mesh->primitives.size();
		for (int i = 0; i < numSurfs; ++i)
		{
			auto& surf = mesh->primitives[i];
			auto it = surf.attributes.find("POSITION");

			if (it == std::end(surf.attributes))
			{
				Error("[ModelManager]: (LoadFromGLTF) %s/%s no position vectors", filename, name);
				return false;
			}

			Accessor const& xyz = model.accessors[it->second];
			Accessor const& idx = model.accessors[surf.indices];
			int const numVerts = xyz.count;
			int const numIndexes = idx.count;
			int surfIndex{};

			modelSurface_t* ms = AllocSurface(numVerts, numIndexes, surfIndex);
			assert(xyz.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

			const unsigned char* pData = 
				model.buffers[model.bufferViews[xyz.bufferView].buffer].data.data() +
				model.bufferViews[xyz.bufferView].byteOffset +
				xyz.byteOffset;

			ptrdiff_t stride = xyz.ByteStride(model.bufferViews[xyz.bufferView]);
			drawVert_t* drawvert = ms->surf.verts;
			for (int j = 0; j < xyz.count; ++j)
			{
				drawvert->SetPos((const float*)pData);
				pData += stride;
				++drawvert;
			}

			pData = 
				model.buffers[model.bufferViews[idx.bufferView].buffer].data.data() +
				model.bufferViews[idx.bufferView].byteOffset +
				idx.byteOffset;
			
			stride = idx.ByteStride(model.bufferViews[idx.bufferView]);
			elementIndex_t* indices = ms->surf.indexes;

			if (idx.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
			{
				for (int j = 0; j < idx.count; ++j)
				{
					*indices = (uint16_t) *((uint32_t*)pData);
					pData += stride;
					++indices;
				}
			}
			else if (idx.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
			{
				for (int j = 0; j < idx.count; ++j)
				{
					*indices = *((uint16_t*)pData);
					pData += stride;
					++indices;
				}
			}
		}

		return false;
	}
}
