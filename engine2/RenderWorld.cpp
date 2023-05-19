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

	RenderWorld::RenderWorld() : map()
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
		if (map) delete map;
		if (filename.empty()) return nullptr;

		DestroyWorld();
		map_mater_idx.clear();

		map = new tinygltf::Model();

		Model& model = *map;

		TinyGLTF loader;
		std::string err, warn;

		bool wasOk = false;

		fs::path fnp(filename);
		if (fnp.extension().string() == ".glb")
		{
			wasOk = loader.LoadBinaryFromFile(map, &err, &warn, filename);
		}
		else
		{
			wasOk = loader.LoadASCIIFromFile(map, &err, &warn, filename);
		}

		if (!wasOk)
		{
			Error("[RenderWorld]: (LoadFromGLTF) %s", err.c_str());
			delete map;

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

		delete map;
		map = nullptr;

		return true;
	}
	void RenderWorld::RenderView(viewDef_t* view)
	{
		using namespace glm;

		for (int i = 0; i < nodes.size(); ++i)
		{
			Node3D* node = nodes[i];
			if (node->GetEntity().GetType() != ENT_MODEL) continue;

			glm::mat4 worldMatrix = node->GetLocalToWorldMatrix();
			glm::mat4 viewMatrix = view->renderView.viewMatrix;
			// W-V-P -> P-V-W
			auto modelViewMatrix = viewMatrix * worldMatrix;

			Bounds entityBounds = node->GetEntity().GetModel()->GetBounds().Transform(modelViewMatrix);
			if (view->frustum.Intersects(entityBounds))
			{
				viewEntity_t* ent = (viewEntity_t*) R_FrameAlloc(sizeof(*ent));
				const auto* model = node->GetEntity().GetModel();

				// entity chain
				ent->next = view->viewEntites;
				view->viewEntites = ent;
				ent->modelMatrix = worldMatrix;
				ent->modelViewMatrix = modelViewMatrix;
				ent->mvp = view->projectionMatrix * ent->modelViewMatrix;

				for (int entSurf = 0; entSurf < model->GetNumSurface(); ++entSurf)
				{
					const auto* surf = model->GetSurface(entSurf);
					const Bounds surfBounds = surf->surf.bounds.Transform(modelViewMatrix);
					if (view->frustum.Intersects(surfBounds))
					{
						drawSurf_t* drawSurf = (drawSurf_t*)R_FrameAlloc(sizeof(*drawSurf));
						drawSurf->frontEndGeo = &surf->surf;
						drawSurf->numIndex = surf->surf.numIndexes;
						drawSurf->indexCache = surf->surf.indexCache;
						drawSurf->vertexCache = surf->surf.vertexCache;
						drawSurf->shader = surf->shader;
						drawSurf->space = ent;
						const vec4 p = ent->modelViewMatrix * vec4(surf->surf.bounds.GetSphere().GetCenter(), 1.0f);
						drawSurf->sort = static_cast<float>((surf->shader->GetId() << 24) - p.z);
						drawSurf->next = ent->surf;
						ent->surf = drawSurf;

						view->numDrawSurfs++;
					}
				}
			}
		}

		//Info("visibe surface count: %d", view->numDrawSurfs);
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

	Bounds RenderWorld::GetBounds() const
	{
		return worldBounds;
	}

	eImageRepeat tiny_map_wrap(int x)
	{
		switch (x)
		{
		case TINYGLTF_TEXTURE_WRAP_REPEAT: return IMR_REPEAT;
		case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE: return IMR_CLAMP_TO_EDGE;
		case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT: return IMR_REPEAT;
		default:
			assert(false);
		}
	}
	void RenderWorld::CreateImagesGLTF()
	{
		if (map == nullptr) return;
		map_image_idx.clear();

		for (int i = 0; i < map->textures.size(); ++i)
		{
			const auto& tex = map->textures[i];
			const auto& img = map->images[tex.source];
			const auto& sam = map->samplers[tex.sampler];

			if (img.bits != 8)
			{
				Info("Image bits: %d", img.bits);
			}

			Image* im = imageManager->AllocImage(img.name);
			map_image_idx.push_back(im->GetId());
			images.insert(im);
			eImageRepeat rs = tiny_map_wrap(sam.wrapS), rt = tiny_map_wrap(sam.wrapT);
			

			//imageOpts_t opts;
			im->opts.autocompress = false;
			im->opts.automipmap = true;
			im->opts.format = img.component == 3 ? IMF_RGB : IMF_RGBA;
			im->opts.maxAnisotropy = 1.0f;
			im->opts.shape = IMS_2D;
			im->opts.sizeX = img.width;
			im->opts.sizeY = img.height;
			im->opts.srgb = false;
			im->SetFilter(IFL_LINEAR_LINEAR, IFL_LINEAR);
			im->SetRepeat(rs, rt);
			im->Bind();
			//im->AllocImage(opts, IFL_LINEAR, IMR_REPEAT);
			im->UpdateImageData(img.width, img.height, 0, 0, 0, 0, img.image.data());
			//Info("img_%s%d uploaded", img.name.c_str(), i);
		}


	}
	void RenderWorld::CreateMaterialsGLTF()
	{
		if (map == nullptr) return;

		map_mater_idx.clear();
		for (int i = 0; i < map->materials.size(); ++i)
		{
			const auto& gmat = map->materials[i];
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
			stage.diffuseScale = glm::make_vec4((double*) gmat.pbrMetallicRoughness.baseColorFactor.data() );
			stage.roughnessScale = static_cast<float>(gmat.pbrMetallicRoughness.roughnessFactor);
			stage.metallicScale = static_cast<float>(gmat.pbrMetallicRoughness.metallicFactor);
			stage.emissiveScale = glm::vec4(glm::make_vec3((double*) gmat.emissiveFactor.data()), 0.0f);

			if (gmat.pbrMetallicRoughness.baseColorTexture.index > -1) {
				stage.images[IMU_DIFFUSE] = imageManager->GetImage(map_image_idx[gmat.pbrMetallicRoughness.baseColorTexture.index]);
			}
			else {
				stage.images[IMU_DIFFUSE] = imageManager->globalImages.whiteImage;
			}
			if (gmat.pbrMetallicRoughness.metallicRoughnessTexture.index > -1)
			{
				stage.images[IMU_AORM] = imageManager->GetImage(map_image_idx[gmat.pbrMetallicRoughness.metallicRoughnessTexture.index]);
			}
			else {
				stage.images[IMU_AORM] = imageManager->globalImages.grayImage;
			}
			if (gmat.emissiveTexture.index > -1)
			{
				stage.images[IMU_EMMISIVE] = imageManager->GetImage(map_image_idx[gmat.emissiveTexture.index]);
			}
			else {
				stage.images[IMU_EMMISIVE] = imageManager->globalImages.blackImage;
			}
			if (gmat.normalTexture.index > -1)
			{
				stage.images[IMU_NORMAL] = imageManager->GetImage(map_image_idx[gmat.normalTexture.index]);
			}
			else {
				stage.images[IMU_NORMAL] = imageManager->globalImages.flatNormal;
			}
		}

	}
	void RenderWorld::CreateNodesGLTF()
	{
		if (map == nullptr) return;
		if (!nodes.empty())
		{
			nodes.clear();
			rootnodes.clear();
		}

		std::vector<RenderModel*> surfcache(map->meshes.size());

		for (int i = 0; i < map->nodes.size(); ++i)
		{
			const auto& gnode = map->nodes[i];

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
						worldBounds.Extend( surfcache[gnode.mesh]->GetBounds().Transform(node->GetLocalToWorldMatrix()) );
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

		for (int i = 0; i < map->nodes.size(); ++i)
		{
			const auto& gnode = map->nodes[i];
			if (gnode.mesh == -1) continue;
			auto* p = nodes[gnode.mesh];
			for (auto child : gnode.children)
			{
				p->AddChild(nodes[child]);
				nodes[child]->SetParent(p);
			}
		}

		int scene = map->defaultScene >= 0 ? map->defaultScene : 0;
		for (int i = 0; i < map->scenes[scene].nodes.size(); ++i)
		{
			rootnodes.push_back(nodes[map->scenes[scene].nodes[i]]);
		}
	}

	RenderModel* RenderWorld::CreateModelGLTF(int mesh)
	{
		const auto& gmesh = map->meshes[mesh];
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

			Accessor const& xyz = map->accessors[it->second];
			Accessor const& idx = map->accessors[surf.indices];
			int const numVerts = xyz.count;
			int const numIndexes = idx.count;
			int surfIndex{};

			modelSurface_t* ms = RM->AllocSurface(numVerts, numIndexes, surfIndex);
			assert(xyz.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

			auto* pData = ACCESSOR_PTR(map, xyz);

			int stride = xyz.ByteStride(map->bufferViews[xyz.bufferView]);
			drawVert_t* drawvert = ms->surf.verts;
			for (int j = 0; j < xyz.count; ++j)
			{
				const float* ff = (const float*)pData;
				ms->surf.bounds << glm::vec3(ff[0], ff[1], ff[2]);
				RM->GetBounds() << glm::vec3(ff[0], ff[1], ff[2]);

				drawvert->SetPos(ff);
				drawvert->SetColor(glm::vec4(1.0f));
				pData += stride;
				++drawvert;
			}

			it = surf.attributes.find("NORMAL");
			if (std::end(surf.attributes) != it)
			{
				Accessor const& normals = map->accessors[it->second];
				auto* pData = ACCESSOR_PTR(map, normals);
				int stride = normals.ByteStride(map->bufferViews[normals.bufferView]);
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
				Accessor const& tangent = map->accessors[it->second];
				auto* pData = ACCESSOR_PTR(map, tangent);

				int stride = tangent.ByteStride(map->bufferViews[tangent.bufferView]);
				drawVert_t* drawvert = ms->surf.verts;
				for (int j = 0; j < tangent.count; ++j)
				{
					const float* f = (const float*)pData;
					drawvert->SetTangent(f);
					pData += stride;
					++drawvert;
				}
			}

			it = surf.attributes.find("TEXCOORD_0");
			if (std::end(surf.attributes) != it)
			{
				Accessor const& uv = map->accessors[it->second];
				assert(uv.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

				auto* pData = ACCESSOR_PTR(map, uv);

				int stride = uv.ByteStride(map->bufferViews[uv.bufferView]);
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
				Accessor const& color = map->accessors[it->second];

				auto const* pData = ACCESSOR_PTR(map, color);
				int stride = color.ByteStride(map->bufferViews[color.bufferView]);
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

			pData = ACCESSOR_PTR(map, idx);

			stride = idx.ByteStride(map->bufferViews[idx.bufferView]);
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