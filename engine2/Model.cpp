#include <filesystem>
#include <utility>
#include <cinttypes>
#include <tiny_gltf.h>

#include "./Math.h"
#include "./Model.h"
#include "./Logger.h"
#include "./GpuTypes.h"

#define ACCESSOR_PTR(model, accessor) \
(unsigned char*)(model.buffers[model.bufferViews[accessor.bufferView].buffer].data.data() + \
model.bufferViews[accessor.bufferView].byteOffset + \
accessor.byteOffset)

namespace jsr {

	using namespace tinygltf;
	namespace fs = std::filesystem;

	RenderModel::RenderModel() : isStatic(true)
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
	
	ModelManager::ModelManager() {}
	ModelManager::~ModelManager() {}


	RenderModel* ModelManager::LoadFromGLTF(const std::string& filename, int index, const std::string& name)
	{
		Model model{};
		TinyGLTF loader{};
		std::string err, warn;

		if (name.empty() && index < 0) return nullptr;

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
			return nullptr;
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
			return nullptr;
		}

		int const numSurfs = mesh->primitives.size();
		
		RenderModel* RM = new RenderModel();
		RM->SetStatic(true);

		for (int i = 0; i < numSurfs; ++i)
		{
			auto& surf = mesh->primitives[i];
			auto it = surf.attributes.find("POSITION");

			if (it == std::end(surf.attributes))
			{
				Error("[ModelManager]: (LoadFromGLTF) %s/%s no position vectors", filename, name);
				return nullptr;
			}

			Accessor const& xyz = model.accessors[it->second];
			Accessor const& idx = model.accessors[surf.indices];
			int const numVerts = xyz.count;
			int const numIndexes = idx.count;
			int surfIndex{};

			modelSurface_t* ms = RM->AllocSurface(numVerts, numIndexes, surfIndex);
			assert(xyz.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

			auto* pData = ACCESSOR_PTR(model, xyz);

			int stride = xyz.ByteStride(model.bufferViews[xyz.bufferView]);
			drawVert_t* drawvert = ms->surf.verts;
			for (int j = 0; j < xyz.count; ++j)
			{
				const float* ff = (const float*)pData;
				ms->surf.bounds << glm::vec3(ff[0], ff[1], ff[2]);
				RM->GetBounds() << glm::vec3(ff[0], ff[1], ff[2]);

				drawvert->SetPos(ff);
				pData += stride;
				++drawvert;
			}

			it = surf.attributes.find("NORMAL");
			if (std::end(surf.attributes) != it)
			{
				Accessor const& normals = model.accessors[it->second];
				auto* pData = ACCESSOR_PTR(model, normals);
				int stride = normals.ByteStride(model.bufferViews[normals.bufferView]);
				drawVert_t* drawvert = ms->surf.verts;
				for (int j = 0; j < normals.count; ++j)
				{
					drawvert->SetNormal((const float*)pData);
					pData += stride;
					++drawvert;
				}
			}

			it = surf.attributes.find("TANGENT");
			if (std::end(surf.attributes) != it)
			{
				Accessor const& tangent = model.accessors[it->second];
				auto* pData = ACCESSOR_PTR(model, tangent);

				int stride = tangent.ByteStride(model.bufferViews[tangent.bufferView]);
				drawVert_t* drawvert = ms->surf.verts;
				for (int j = 0; j < tangent.count; ++j)
				{
					drawvert->SetTangent((const float*)pData);
					pData += stride;
					++drawvert;
				}
			}

			it = surf.attributes.find("TEXCOORD_0");
			if (std::end(surf.attributes) != it)
			{
				Accessor const& uv = model.accessors[it->second];
				assert(uv.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

				auto* pData = ACCESSOR_PTR(model, uv);

				int stride = uv.ByteStride(model.bufferViews[uv.bufferView]);
				drawVert_t* drawvert = ms->surf.verts;
				for (int j = 0; j < uv.count; ++j)
				{
					drawvert->SetUV((const float*)pData);
					pData += stride;
					++drawvert;
				}
			}

			it = surf.attributes.find("COLOR_0");
			if (std::end(surf.attributes) != it)
			{
				Accessor const& color = model.accessors[it->second];

				auto const* pData = ACCESSOR_PTR(model, color);
				int stride = color.ByteStride(model.bufferViews[color.bufferView]);
				drawVert_t* drawvert = ms->surf.verts;
				int n = color.type == TINYGLTF_TYPE_VEC4 ? 4 : 3;
				
				for (int j = 0; j < color.count; ++j)
				{
					glm::vec4 aColor{ 0.0f,0.0f,0.0f,1.0f };
					for (int k = 0; k < n; ++k)
					{
						switch (color.componentType)
						{
						case TINYGLTF_COMPONENT_TYPE_FLOAT:
							aColor[k] = *( ( (float*)pData ) + k);
							break;
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
							aColor[k] = unorm16ToFloat( *( ((unsigned short*)pData) + k) );
							break;
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
							aColor[k] = unorm8ToFloat( *( ((unsigned char*)pData) + k) );
							break;
						}
					}
					drawvert->SetColor(aColor);
					pData += stride;
					++drawvert;
				}
			}

			pData = ACCESSOR_PTR(model, idx);
			
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

		return RM;
	}

	void RenderModel::UpdateSurfaceCache()
	{

	}
	void RenderModel::SetStatic(bool b)
	{
		isStatic = b;
	}
}
