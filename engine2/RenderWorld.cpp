#include <tiny_gltf.h>
#include <memory>
#include <filesystem>
#include <glm/gtx/matrix_decompose.hpp>

#include "./Math.h"
#include "./RenderSystem.h"
#include "./RenderWorld.h"
#include "./ImageManager.h"
#include "./Material.h"
#include "./Entity3D.h"
#include "./Node3D.h"
#include "./Logger.h"

#define ACCESSOR_PTR(model, accessor) \
(unsigned char*)(model->buffers[model->bufferViews[accessor.bufferView].buffer].data.data() + \
model->bufferViews[accessor.bufferView].byteOffset + \
accessor.byteOffset)

namespace jsr {

	using namespace tinygltf;
	namespace fs = std::filesystem;

	RenderWorld::RenderWorld() : maptoload()
	{
		vertecCache		= renderSystem.vertexCache;
		imageManager	= renderSystem.imageManager;
		modelManager	= renderSystem.modelManager;
		materialManager = renderSystem.materialManager;
	}

	RenderWorld::~RenderWorld()
	{
		DestroyWorld();
	}

	bool RenderWorld::LoadMapFromGLTF(const std::string& filename)
	{
		if (maptoload) delete maptoload;
		if (filename.empty()) return nullptr;

		DestroyWorld();
		map_mater_idx.clear();

		maptoload = new tinygltf::Model();

		Model& model = *maptoload;

		TinyGLTF loader;
		std::string err, warn;

		bool wasOk = false;

		fs::path fnp(filename);
		if (fnp.extension().string() == ".glb")
		{
			wasOk = loader.LoadBinaryFromFile(maptoload, &err, &warn, filename);
		}
		else
		{
			wasOk = loader.LoadASCIIFromFile(maptoload, &err, &warn, filename);
		}

		if (!wasOk)
		{
			Error("[RenderWorld]: (LoadFromGLTF) %s", err.c_str());
			delete maptoload;

			return false;
		}

		if (!warn.empty())
		{
			Info("[RenderWorld]: (LoadFromGLTF) %s", warn.c_str());
		}

		// checking
		if (model.scenes[0].nodes.empty()) 
		{ 
			Error("[RenderWorld]: (LoadFromGLTF) scene has zero node!");
			return false; 
		}

		CreateImagesGLTF();
		CreateMaterialsGLTF();
		// CreateLightsGLTF
		CreateNodesGLTF();

		delete maptoload;
		maptoload = nullptr;

		return true;
	}
	void RenderWorld::RenderView(viewDef_t* view)
	{
		for (int i = 0; i < nodes.size(); ++i)
		{
			Node3D* node = nodes[i];
			if (node->GetEntity().GetType() != ENT_MODEL) continue;

			glm::mat4 worldMatrix = node->GetLocalToWorldMatrix();
			glm::mat4 viewMatrix = glm::mat4(view->renderView.viewaxis);
			viewMatrix[3] = glm::vec4(view->renderView.vieworg, 1.0f);

			auto modelViewMatrix = viewMatrix * worldMatrix;

			Bounds entityBounds = node->GetEntity().GetModel()->GetBounds();
			Frustum f(view->projectionMatrix, modelViewMatrix);
			if (f.BoundsTest(entityBounds))
			{
				viewEntity_t* ent = (viewEntity_t*) R_FrameAlloc(sizeof(*ent));
				const auto* model = node->GetEntity().GetModel();

				ent->next = view->viewEntites;
				view->viewEntites = ent;
				ent->modelMatrix = worldMatrix;
				ent->modelViewMatrix = modelViewMatrix;
				ent->mvp = view->projectionMatrix * ent->modelViewMatrix;

				for (int entSurf = 0; entSurf < model->GetNumSurface(); ++entSurf)
				{
					const auto* surf = model->GetSurface(entSurf);
					if (f.BoundsTest(surf->surf.bounds))
					{
						drawSurf_t* drawSurf = (drawSurf_t*)R_FrameAlloc(sizeof(*drawSurf));
						drawSurf->frontEndGeo = &surf->surf;
						drawSurf->numIndex = surf->surf.numIndexes;
						drawSurf->indexCache = surf->surf.indexCache;
						drawSurf->vertexCache = surf->surf.vertexCache;
						drawSurf->shader = surf->shader;
						drawSurf->space = ent;
						glm::vec4 p = ent->modelViewMatrix * glm::vec4(surf->surf.bounds.GetSphere().GetCenter(), 1.0f);
						drawSurf->sort = p.z;
						ent->surf = drawSurf;

						view->numDrawSurfs++;
					}
				}
			}
		}
		Info("visibe surface count: %d", view->numDrawSurfs);
	}
	void RenderWorld::DestroyWorld()
	{
		
		for (auto* node3d : nodes) 
		{ 
			delete node3d; 
		}

		for (auto* e : materials) { materialManager->RemoveMaterial(e); }
		for (auto* e : images) { imageManager->RemoveImage(e); }
		for (auto* e : models) { modelManager->RemoveModel(e); }

		nodes.clear();
		rootnodes.clear();
		materials.clear();
		models.clear();
		images.clear();
	}

	void RenderWorld::CreateImagesGLTF()
	{
		if (maptoload == nullptr) return;
		map_image_idx.clear();

		for (int i = 0; i < maptoload->images.size(); ++i)
		{
			const auto& img = maptoload->images[i];
			if (img.bits != 8)
			{
				Info("Image bits: %d", img.bits);
			}

			Image* im = imageManager->AllocImage(img.name);
			map_image_idx.push_back(im->GetId());
			images.insert(im);

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


	}
	void RenderWorld::CreateMaterialsGLTF()
	{
		if (maptoload == nullptr) return;

		map_mater_idx.clear();
		for (int i = 0; i < maptoload->materials.size(); ++i)
		{
			const auto& gmat = maptoload->materials[i];
			Material* mat = materialManager->CreateMaterial(gmat.name);
			materials.insert(mat);

			map_mater_idx.push_back(mat->GetId());

			eCoverage cov = gmat.alphaMode == "OPAQUE" ? COVERAGE_SOLID : (gmat.alphaMode == "BLEND" ? COVERAGE_BLEND : COVERAGE_MASK);

			stage_t& stage = mat->GetStage(STAGE_DEBUG);
			stage.alphaCutoff = gmat.alphaCutoff;
			stage.coverage = cov;
			stage.shader = PRG_TEXTURED;
			stage.enabled = true;
			stage.cullMode = gmat.doubleSided ? CULL_NONE : CULL_BACK;
			stage.type = STAGE_DEBUG;
			if (gmat.pbrMetallicRoughness.baseColorTexture.index > -1) {
				const auto& tex = maptoload->textures[gmat.pbrMetallicRoughness.baseColorTexture.index];
				stage.images[IMU_DIFFUSE] = imageManager->GetImage(map_image_idx[tex.source]);
			}
			else {
				stage.images[IMU_DIFFUSE] = imageManager->globalImages.whiteImage;
			}
			if (gmat.pbrMetallicRoughness.metallicRoughnessTexture.index > -1)
			{
				const auto& tex = maptoload->textures[gmat.pbrMetallicRoughness.metallicRoughnessTexture.index];
				stage.images[IMU_AORM] = imageManager->GetImage(map_image_idx[tex.source]);
			}
			else {
				stage.images[IMU_AORM] = imageManager->globalImages.grayImage;
			}
			if (gmat.emissiveTexture.index > -1)
			{
				const auto& tex = maptoload->textures[gmat.emissiveTexture.index];
				stage.images[IMU_EMMISIVE] = imageManager->GetImage(map_image_idx[tex.source]);
			}
			else {
				stage.images[IMU_EMMISIVE] = imageManager->globalImages.blackImage;
			}
			if (gmat.normalTexture.index > -1)
			{
				const auto& tex = maptoload->textures[gmat.normalTexture.index];
				stage.images[IMU_NORMAL] = imageManager->GetImage(map_image_idx[tex.source]);
			}
			else {
				stage.images[IMU_NORMAL] = imageManager->globalImages.flatNormal;
			}
		}

	}
	void RenderWorld::CreateNodesGLTF()
	{
		if (maptoload == nullptr) return;
		if (!nodes.empty())
		{
			nodes.clear();
			rootnodes.clear();
		}

		std::vector<RenderModel*> surfcache(maptoload->meshes.size());

		for (int i = 0; i < maptoload->nodes.size(); ++i)
		{
			const auto& gnode = maptoload->nodes[i];

			Node3D* node = new Node3D();
			nodes.push_back(node);

			if (!gnode.matrix.empty())
			{
				glm::vec3 scale{}, translation{}, skew{};
				glm::vec4 persp{};
				glm::quat orient{};
				std::vector<float> m4(16);
				for (int i = 0; i < 16; ++i) m4[i] = (float)gnode.matrix[i];

				glm::mat4 m = glm::make_mat4(m4.data());
				if (glm::decompose(m, scale, orient, translation, skew, persp))
				{
					node->SetOrigin(translation);
					node->SetDir(orient);
					node->SetScale(scale);
				}
			}
			else
			{
				if (!gnode.translation.empty()) {
					node->SetOrigin(glm::vec3((float)gnode.translation[0], (float)gnode.translation[1], (float)gnode.translation[2]));
				}
				if (!gnode.scale.empty()) {
					node->SetScale(glm::vec3((float)gnode.scale[0], (float)gnode.scale[1], (float)gnode.scale[2]));
				}
				if (!gnode.rotation.empty()) {
					node->SetDir(glm::quat((float)gnode.rotation[3], (float)gnode.rotation[0], (float)gnode.rotation[1], (float)gnode.rotation[2]));
				}
			}

			if (gnode.mesh > -1)
			{
				if (surfcache[gnode.mesh] == nullptr)
				{
					surfcache[gnode.mesh] = CreateModelGLTF(gnode.mesh);
					if (surfcache[gnode.mesh] == nullptr)
					{
						Error("Mesh not loaded !");
					}
					else
					{
						surfcache[gnode.mesh]->UpdateSurfaceCache();
						models.insert(surfcache[gnode.mesh]);
						worldBounds << surfcache[gnode.mesh]->GetBounds();
					}
				}

				node->GetEntity().SetType(ENT_MODEL);
				node->GetEntity().SetValue(surfcache[gnode.mesh]);
			}
			else
			{
				node->GetEntity().SetType(ENT_EMPTY);
			}
		}

		for (int i = 0; i < maptoload->nodes.size(); ++i)
		{
			const auto& gnode = maptoload->nodes[i];
			if (gnode.mesh == -1) continue;
			auto* p = nodes[gnode.mesh];
			for (auto child : gnode.children)
			{
				p->AddChild(nodes[child]);
				nodes[child]->SetParent(p);
			}
		}

		int scene = maptoload->defaultScene >= 0 ? maptoload->defaultScene : 0;
		for (int i = 0; i < maptoload->scenes[scene].nodes.size(); ++i)
		{
			rootnodes.push_back(nodes[maptoload->scenes[scene].nodes[i]]);
		}
	}

	RenderModel* RenderWorld::CreateModelGLTF(int mesh)
	{
		const auto& gmesh = maptoload->meshes[mesh];
		const int numSurfs = gmesh.primitives.size();

		RenderModel* RM = modelManager->CreateModel(gmesh.name);

		RM->SetStatic(true);

		for (int i = 0; i < numSurfs; ++i)
		{
			auto& surf = gmesh.primitives[i];
			if (surf.mode != TINYGLTF_MODE_TRIANGLES) continue;

			auto it = surf.attributes.find("POSITION");

			if (it == std::end(surf.attributes))
			{
				Error("[RenderWorld]: (LoadFromGLTF) %s no position vectors", gmesh.name);
				return nullptr;
			}

			Accessor const& xyz = maptoload->accessors[it->second];
			Accessor const& idx = maptoload->accessors[surf.indices];
			int const numVerts = xyz.count;
			int const numIndexes = idx.count;
			int surfIndex{};

			modelSurface_t* ms = RM->AllocSurface(numVerts, numIndexes, surfIndex);
			assert(xyz.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

			auto* pData = ACCESSOR_PTR(maptoload, xyz);

			int stride = xyz.ByteStride(maptoload->bufferViews[xyz.bufferView]);
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
				Accessor const& normals = maptoload->accessors[it->second];
				auto* pData = ACCESSOR_PTR(maptoload, normals);
				int stride = normals.ByteStride(maptoload->bufferViews[normals.bufferView]);
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
				Accessor const& tangent = maptoload->accessors[it->second];
				auto* pData = ACCESSOR_PTR(maptoload, tangent);

				int stride = tangent.ByteStride(maptoload->bufferViews[tangent.bufferView]);
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
				Accessor const& uv = maptoload->accessors[it->second];
				assert(uv.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

				auto* pData = ACCESSOR_PTR(maptoload, uv);

				int stride = uv.ByteStride(maptoload->bufferViews[uv.bufferView]);
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
				Accessor const& color = maptoload->accessors[it->second];

				auto const* pData = ACCESSOR_PTR(maptoload, color);
				int stride = color.ByteStride(maptoload->bufferViews[color.bufferView]);
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

			pData = ACCESSOR_PTR(maptoload, idx);

			stride = idx.ByteStride(maptoload->bufferViews[idx.bufferView]);
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

			jsr::Material* jmat = renderSystem.materialManager->GetMaterial(map_mater_idx[surf.material]);
			ms->shader = jmat;
			ms->surf.topology = TP_TRIANGLES;
		}
		return RM;
	}
}