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
		for (auto* surf : surfs)
		{
			if (surf->surf.verts) { MemFree( surf->surf.verts ); }
			if (surf->surf.indexes) { MemFree( surf->surf.indexes ); }
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
		surf->surf.verts = (drawVert_t*) MemAlloc(numVerts * sizeof(drawVert_t));
		if (numIndexes > 0)
		{
			surf->surf.indexes = (elementIndex_t*) MemAlloc(sizeof(elementIndex_t) * numIndexes);
		}
		surf->id = jsr::GetTimeMillisecond() + GetUniqId();

		return surf;
	}
	
	ModelManager::ModelManager() {}
	ModelManager::~ModelManager() 
	{
		for (auto* model : models)
		{
			if (model) delete model;
		}
	}


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
		std::vector<int> image_index;
		std::vector<int> material_index;
		
		image_index.reserve(model.images.size());
		material_index.reserve(model.materials.size());

		for (int i = 0; i < model.images.size(); ++i)
		{
			const auto& img = model.images[i];
			if (img.bits != 8) continue;
			
			Image* im = renderSystem.imageManager->AllocImage(img.name);
			image_index.push_back(im->GetId());

			//imageOpts_t opts;
			im->opts.autocompress = false;
			im->opts.automipmap = true;
			im->opts.format = img.component == 3 ? IMF_RGB : IMF_RGBA;
			im->opts.maxAnisotropy = 1.0f;
			im->opts.shape = IMS_2D;
			im->opts.sizeX = img.width;
			im->opts.sizeY = img.height;
			im->Bind();
			//im->AllocImage(opts, IFL_LINEAR, IMR_REPEAT);
			im->UpdateImageData(img.width, img.height, 0, 0, 0, 0, img.image.data());
			//Info("img_%s%d uploaded", img.name.c_str(), i);
		}

		for (int i = 0; i < model.materials.size(); ++i)
		{
			const auto& gmat = model.materials[i];
			Material* mat = renderSystem.materialManager->CreateMaterial(gmat.name);
			material_index.push_back(mat->GetId());

			eCoverage cov = gmat.alphaMode == "OPAQUE" ? COVERAGE_SOLID : (gmat.alphaMode == "BLEND" ? COVERAGE_BLEND : COVERAGE_MASK);

			stage_t& stage = mat->GetStage(STAGE_DEBUG);
			stage.alphaCutoff = gmat.alphaCutoff;
			stage.coverage = cov;
			stage.shader = PRG_TEXTURED;
			stage.enabled = true;
			stage.cullMode = gmat.doubleSided ? CULL_NONE : CULL_BACK;
			stage.type = STAGE_DEBUG;
			if (gmat.pbrMetallicRoughness.baseColorTexture.index > -1) {
				stage.images[IMU_DIFFUSE] = renderSystem.imageManager->GetImage(image_index[ gmat.pbrMetallicRoughness.baseColorTexture.index ]);
			}
			else {
				stage.images[IMU_DIFFUSE] = renderSystem.imageManager->globalImages.whiteImage;
			}
			if (gmat.pbrMetallicRoughness.metallicRoughnessTexture.index > -1)
			{
				stage.images[IMU_AORM] = renderSystem.imageManager->GetImage(image_index[ gmat.pbrMetallicRoughness.metallicRoughnessTexture.index ]);
			}
			else {
				stage.images[IMU_AORM] = renderSystem.imageManager->globalImages.grayImage;
			}
			if (gmat.emissiveTexture.index > -1)
			{
				stage.images[IMU_EMMISIVE] = renderSystem.imageManager->GetImage(image_index[ gmat.emissiveTexture.index ]);
			}
			else {
				stage.images[IMU_EMMISIVE] = renderSystem.imageManager->globalImages.blackImage;
			}
			if (gmat.normalTexture.index > -1)
			{
				stage.images[IMU_NORMAL] = renderSystem.imageManager->GetImage(image_index[ gmat.normalTexture.index ]);
			}
			else {
				stage.images[IMU_NORMAL] = renderSystem.imageManager->globalImages.flatNormal;
			}
		}

		for (int i = 0; i < numSurfs; ++i)
		{
			auto& surf = mesh->primitives[i];
			if (surf.mode != TINYGLTF_MODE_TRIANGLES) continue;

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
							aColor[k] = *(((float*)pData) + k);
							break;
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
							aColor[k] = unorm16ToFloat(*(((unsigned short*)pData) + k));
							break;
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
							aColor[k] = unorm8ToFloat(*(((unsigned char*)pData) + k));
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
					*indices = (uint16_t) * ((uint32_t*)pData);
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

			jsr::Material* jmat = renderSystem.materialManager->GetMaterial(material_index[surf.material]);
			ms->shader = jmat;
			ms->surf.topology = TP_TRIANGLES;
		}

		return RM;
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
			return models[idx];
		}
		models.push_back(new RenderModel());
		RenderModel* model = models.back();
		model->id = models.size() - 1;
		model->SetStatic(true);

		return model;
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
			for (auto* surf : surfs)
			{
				if (surf->surf.vertexCache == 0) {
					surf->surf.vertexCache = renderSystem.vertexCache->AllocStaticVertex(surf->surf.verts, sizeof(drawVert_t) * surf->surf.numVerts);
				}
				if (surf->surf.indexCache == 0) {
					surf->surf.indexCache = renderSystem.vertexCache->AllocStaticIndex(surf->surf.indexes, (15UL + sizeof(elementIndex_t) * surf->surf.numIndexes) & ~15UL);
				}
				surf->surf.gpuResident = true;
				MemFree16(surf->surf.verts); surf->surf.verts = nullptr;
				MemFree16(surf->surf.indexes); surf->surf.indexes = nullptr;

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
			for (auto* surf : surfs)
			{
				surf->surf.vertexCache = renderSystem.vertexCache->AllocTransientVertex(surf->surf.verts, sizeof(drawVert_t) * surf->surf.numVerts);
				surf->surf.indexCache = renderSystem.vertexCache->AllocTransientIndex(surf->surf.indexes, (15UL + sizeof(elementIndex_t) * surf->surf.numIndexes) & ~15UL) ;
			}
		}
	}
	void RenderModel::SetStatic(bool b)
	{
		isStatic = b;
	}
	modelSurface_t::~modelSurface_t()
	{
		//if (shader) delete shader;
	}
}
