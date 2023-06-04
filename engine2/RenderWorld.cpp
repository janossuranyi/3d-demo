#include <tiny_gltf.h>
#include <memory>
#include <filesystem>
#include <glm/gtx/matrix_decompose.hpp>

#include "./Math.h"
#include "./RenderSystem.h"
#include "./RenderWorld.h"
#include "./RenderCommon.h"
#include "./ImageManager.h"
#include "./Material.h"
#include "./Entity3D.h"
#include "./Node3D.h"
#include "./Logger.h"
#include "./Light.h"

#define ACCESSOR_PTR(model, accessor) \
(unsigned char*)(model->buffers[model->bufferViews[accessor.bufferView].buffer].data.data() + \
model->bufferViews[accessor.bufferView].byteOffset + \
accessor.byteOffset)

namespace jsr {

	using namespace tinygltf;
	namespace fs = std::filesystem;

	const float BLENDER_LIGHT_POWER_FACTOR = 1.0f/100.0f;

	RenderWorld::RenderWorld() : gltf_state()
	{
		vertexCache = renderSystem.vertexCache;
		imageManager = renderSystem.imageManager;
		modelManager = renderSystem.modelManager;
		materialManager = renderSystem.materialManager;
		exposure = 1.0f;
	}

	RenderWorld::~RenderWorld()
	{
		DestroyWorld();
	}

	bool RenderWorld::LoadMapFromGLTF(const std::string& filename)
	{
		if (gltf_state) delete gltf_state;
		if (filename.empty()) return nullptr;

		DestroyWorld();

		gltf_state = new gltf_state_t();

		Model* model = &gltf_state->map;

		TinyGLTF loader;
		std::string err, warn;

		bool wasOk = false;

		fs::path fnp(filename);
		if (fnp.extension().string() == ".glb")
		{
			wasOk = loader.LoadBinaryFromFile(model, &err, &warn, filename);
		}
		else
		{
			wasOk = loader.LoadASCIIFromFile(model, &err, &warn, filename);
		}

		if (!wasOk)
		{
			Error("[RenderWorld]: (LoadFromGLTF) %s", err.c_str());
			delete gltf_state;
			gltf_state = nullptr;
			return false;
		}

		if (!warn.empty())
		{
			Info("[RenderWorld]: (LoadFromGLTF) %s", warn.c_str());
		}

		// checking
		if (model->scenes[0].nodes.empty()) 
		{ 
			Error("[RenderWorld]: (LoadFromGLTF) scene has zero node!");
			return false; 
		}

		CreateImagesGLTF();
		CreateMaterialsGLTF();
		CreateLightsGLTF();
		CreateModelsGLTF();
		CreateNodesGLTF();

		delete gltf_state;
		gltf_state = nullptr;

		return true;
	}
	void RenderWorld::LoadModelsFromGLTF(const std::string& filename)
	{
		if (gltf_state) delete gltf_state;
		if (filename.empty()) return;

		gltf_state = new gltf_state_t();

		Model* model = &gltf_state->map;

		TinyGLTF loader;
		std::string err, warn;

		bool wasOk = false;

		fs::path fnp(filename);
		if (fnp.extension().string() == ".glb")
		{
			wasOk = loader.LoadBinaryFromFile(model, &err, &warn, filename);
		}
		else
		{
			wasOk = loader.LoadASCIIFromFile(model, &err, &warn, filename);
		}

		if (!wasOk)
		{
			Error("[RenderWorld]: (LoadModelsFromGLTF) %s", err.c_str());
			delete gltf_state;
			gltf_state = nullptr;
			return;
		}

		if (!warn.empty())
		{
			Info("[RenderWorld]: (LoadModelsFromGLTF) %s", warn.c_str());
		}

		// checking
		if (model->meshes.empty())
		{
			Error("[RenderWorld]: (LoadModelsFromGLTF) scene have no models!");
			return;
		}

		CreateImagesGLTF();
		CreateMaterialsGLTF();
		// CreateLightsGLTF
		CreateModelsGLTF();

		delete gltf_state;
		gltf_state = nullptr;	
	}

	struct
	{
		bool operator()(const drawSurf_t* a, const drawSurf_t* b) const { return (uint32)a->sort < (uint32)b->sort; }
	} drawSurfLess;

	void RenderWorld::RenderView(viewDef_t* view)
	{
		using namespace glm;

		for (int i = 0; i < rootnodes.size(); ++i)
		{
			Node3D* node = nodes[i];
			int numChildren = node->GetNumChildren();
			Node3D** children = node->GetChildren();
			for (int k = 0; k < numChildren; ++k)
			{
				RenderNode(children[k], view);
			}
			RenderNode(node, view);
		}
		//Info("visibe surface count: %d", view->numDrawSurfs);

		if (view->numDrawSurfs)
		{
			// allocate drawSurf pointers
			view->drawSurfs = (const drawSurf_t**)R_FrameAlloc(view->numDrawSurfs * sizeof(view->drawSurfs));
			int i = 0;
			for (const auto* ent = view->viewEntites; ent != nullptr; ent = ent->next)
			{
				for (const auto* drawSurf = ent->surf; drawSurf != nullptr; drawSurf = drawSurf->next)
				{
					view->drawSurfs[i++] = drawSurf;
				}
			}

			std::sort(view->drawSurfs, view->drawSurfs + view->numDrawSurfs, drawSurfLess);
			{
				drawViewCommand_t* cmd = (drawViewCommand_t*)R_GetCommandBuffer(sizeof(*cmd));
				cmd->command = RC_DRAW_VIEW;
				cmd->view = view;
			}
		}
	}

	void RenderWorld::InsertNode(const std::string& name, RenderModel* model, const glm::vec3& pos)
	{
		Node3D* node = new Node3D(name);
		nodes.push_back(node);
		rootnodes.push_back(node);
		node->SetOrigin(pos);
		node->GetEntity().SetType(ENT_MODEL);
		node->GetEntity().SetValue(model);
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
		for (auto* e : lights) { delete e; }

		nodes.clear();
		rootnodes.clear();
		materials.clear();
		models.clear();
		images.clear();
		lights.clear();
	}

	Node3D* RenderWorld::GetByName(const std::string& name)
	{
		for (auto* n : nodes)
		{
			if (n->GetName() == name) return n;
		}

		return nullptr;
	}

	Bounds RenderWorld::GetBounds() const
	{
		return worldBounds;
	}

	float RenderWorld::GetExposure() const
	{
		return exposure;
	}

	void RenderWorld::SetExposure(float x)
	{
		exposure = x;
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
		if (gltf_state == nullptr) return;
		gltf_state->map_image_idx.clear();

		Model* map = &gltf_state->map;
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
			gltf_state->map_image_idx.push_back(im->GetId());
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
		if (gltf_state == nullptr) return;

		gltf_state->map_mater_idx.clear();
		Model* map = &gltf_state->map;
		for (int i = 0; i < map->materials.size(); ++i)
		{
			const auto& gmat = map->materials[i];
			Material* mat = materialManager->CreateMaterial(gmat.name);
			materials.insert(mat);

			gltf_state->map_mater_idx.push_back(mat->GetId());

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

			if (gmat.name.find("noshadow") == std::string::npos)
			{
				stage_t& shadow = mat->GetStage(STAGE_SHADOW);
				shadow.shader = PRG_ZPASS;
				shadow.coverage = cov;
				shadow.enabled = true;
				shadow.type = STAGE_SHADOW;
			}

			if (gmat.pbrMetallicRoughness.baseColorTexture.index > -1) {
				stage.images[IMU_DIFFUSE] = imageManager->GetImage(gltf_state->map_image_idx[gmat.pbrMetallicRoughness.baseColorTexture.index]);
			}
			else {
				stage.images[IMU_DIFFUSE] = globalImages.whiteImage;
			}
			if (gmat.pbrMetallicRoughness.metallicRoughnessTexture.index > -1)
			{
				stage.images[IMU_AORM] = imageManager->GetImage(gltf_state->map_image_idx[gmat.pbrMetallicRoughness.metallicRoughnessTexture.index]);
			}
			else {
				stage.images[IMU_AORM] = globalImages.grayImage;
			}
			if (gmat.emissiveTexture.index > -1)
			{
				stage.images[IMU_EMMISIVE] = imageManager->GetImage(gltf_state->map_image_idx[gmat.emissiveTexture.index]);
			}
			else {
				stage.images[IMU_EMMISIVE] = globalImages.whiteImage;
			}
			if (gmat.normalTexture.index > -1)
			{
				stage.images[IMU_NORMAL] = imageManager->GetImage(gltf_state->map_image_idx[gmat.normalTexture.index]);
			}
			else {
				stage.images[IMU_NORMAL] = globalImages.flatNormal;
			}
		}

	}
	void RenderWorld::CreateNodesGLTF()
	{
		if (gltf_state == nullptr) return;
		if (!nodes.empty())
		{
			nodes.clear();
			rootnodes.clear();
		}

		Model* map = &gltf_state->map;
		std::vector<int> gltfNodeToLocal;
		gltfNodeToLocal.resize(map->nodes.size());

		for (int i = 0; i < map->nodes.size(); ++i)
		{
			const auto& gnode = map->nodes[i];

			Node3D* node = new Node3D();
			nodes.push_back(node);
			gltfNodeToLocal[i] = (int)nodes.size() - 1;
			node->SetName(gnode.name);

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
				models.insert(gltf_state->map_models[gnode.mesh]);
				worldBounds.Extend(gltf_state->map_models[gnode.mesh]->GetBounds().Transform(node->GetLocalToWorldMatrix()) );
				node->GetEntity().SetType(ENT_MODEL);
				node->GetEntity().SetValue(gltf_state->map_models[gnode.mesh]);
			}
			else if (gnode.extensions.count("KHR_lights_punctual"))
			{
				node->GetEntity().SetType(ENT_LIGHT);
				auto it = gnode.extensions.find("KHR_lights_punctual");
				int idx = it->second.Get("light").GetNumberAsInt();
				node->GetEntity().SetValue( lights[ gltf_state->map_light_idx[ idx ] ] );
			}
			else
			{
				node->GetEntity().SetType(ENT_EMPTY);
			}
		}

		for (int i = 0; i < map->nodes.size(); ++i)
		{
			const auto& gnode = map->nodes[i];
			Node3D* n = nodes[gltfNodeToLocal[i]];
			for (auto child : gnode.children)
			{
				Node3D* ch = nodes[gltfNodeToLocal[child]];
				n->AddChild(ch);
				ch->SetParent(n);
			}
		}

		int scene = map->defaultScene >= 0 ? map->defaultScene : 0;
		for (int i = 0; i < map->scenes[scene].nodes.size(); ++i)
		{
			rootnodes.push_back(nodes[ gltfNodeToLocal[ map->scenes[scene].nodes[i] ] ]);
		}
	}

	void RenderWorld::CreateModelsGLTF()
	{
		if (gltf_state == nullptr) return;
		
		const Model& map = gltf_state->map;
		gltf_state->map_models.resize(map.meshes.size());

		for (int i = 0; i < map.meshes.size(); ++i)
		{
			gltf_state->map_models[i] = CreateModelGLTF(i);
			gltf_state->map_models[i]->UpdateSurfaceCache();
		}
	}

	void RenderWorld::CreateLightsGLTF()
	{
		if (gltf_state == nullptr) return;
		const Model& map = gltf_state->map;
		gltf_state->map_light_idx.resize(map.lights.size());

		for (int i = 0; i < map.lights.size(); ++i)
		{
			const auto& e = map.lights[i];
			eLightType ltype = LIGHT_POINT;
			if (e.type == "pdirectional")	ltype = LIGHT_DIRECTED;
			else if (e.type == "spot")		ltype = LIGHT_SPOT;

			auto* light = lights.emplace_back(new Light(ltype));
			light->SetId(lights.size() - 1);
			gltf_state->map_light_idx[i] = light->GetId();

			// Ln = (683 * watt)  / ( 4 * math.pi )
			// watt = Ln * 4PI / 683
			float watts = .05f * (static_cast<float>(e.intensity) * glm::pi<float>() * 4.0f) / 683.0f;
			light->SetName(e.name);
			light->opts.color = lightColor_t{ glm::make_vec3((double*)e.color.data()), watts};
			light->opts.expAttn = renderGlobals.defaultExpAttn;
			light->opts.linearAttn = 0.0f;
			light->SetShader(PRG_DEFERRED_LIGHT);
			if (e.range > 0.0)
			{
				light->opts.range = static_cast<float>(e.range);
			}
			else
			{
				light->opts.CalculateRange();
			}

			if (ltype == LIGHT_SPOT) 
			{
				light->opts.outerConeAngle = e.spot.outerConeAngle;
				light->opts.innerConeAngle = e.spot.innerConeAngle;
			}
		}
	}

	RenderModel* RenderWorld::CreateModelGLTF(int mesh)
	{
		const auto& gmesh = gltf_state->map.meshes[mesh];
		const int numSurfs = gmesh.primitives.size();

		RenderModel* RM = modelManager->CreateModel(gmesh.name);

		RM->SetStatic(true);
		Model* map = &gltf_state->map;
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

			tinygltf::Accessor const& xyz = map->accessors[it->second];
			tinygltf::Accessor const& idx = map->accessors[surf.indices];
			int const numVerts = xyz.count;
			int const numIndexes = idx.count;
			int surfIndex = RM->AllocSurface(numVerts, numIndexes);

			modelSurface_t* ms = RM->GetSurface(surfIndex);
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
				tinygltf::Accessor const& normals = map->accessors[it->second];
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
				tinygltf::Accessor const& tangent = map->accessors[it->second];
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
				tinygltf::Accessor const& uv = map->accessors[it->second];
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
				tinygltf::Accessor const& color = map->accessors[it->second];

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

			if (surf.material > -1)
			{
				jsr::Material* jmat = renderSystem.materialManager->GetMaterial(gltf_state->map_mater_idx[surf.material]);
				ms->shader = jmat;
				ms->surf.topology = TP_TRIANGLES;
			}
			else 
			{
				ms->shader = renderSystem.defaultMaterial;
				ms->surf.topology = TP_TRIANGLES;
			}
		}
		return RM;
	}

	void RenderWorld::RenderNode(Node3D* node, viewDef_t* view)
	{
		using namespace glm;

		const mat4& viewMatrix = view->renderView.viewMatrix;
		VertexCache& vc = *renderSystem.vertexCache;

		alignas(16) uboFreqHighVert_t fastvert {};
		alignas(16) uboFreqHighFrag_t fastfrag {};

		if (node->GetEntity().IsLight())
		{
			Light* light = node->GetEntity().GetLight();

			mat4 modelMatrix = node->GetLocalToWorldMatrix();
			vec4 origin = modelMatrix[3];
			mat4 worldMatrix = translate(mat4(1.0f), vec3(origin));
			worldMatrix = scale(worldMatrix, vec3(light->opts.range));

			mat4 modelViewMatrix = viewMatrix * worldMatrix;

			Bounds lightBounds = Bounds(-vec3(light->opts.range / 2.0f), vec3(light->opts.range / 2.0f)).Transform(modelViewMatrix);
			if (view->frustum.Intersects2(lightBounds))
			{

				viewLight_t* e = (viewLight_t*)R_FrameAlloc(sizeof(*e));
				e->next = view->viewLights;
				e->origin = view->renderView.viewMatrix * origin;
				e->axis = mat3(modelMatrix);
				e->range = light->opts.range;
				e->shader = light->GetShader();
				e->color = light->opts.color.color;
				e->remove = false;
				view->viewLights = e;

				fastvert.WVPMatrix = view->projectionMatrix * view->renderView.viewMatrix * worldMatrix;
				fastvert.localToWorldMatrix = worldMatrix;
				fastfrag.lightColor = light->opts.color.color;
				fastfrag.lightAttenuation = vec4(e->range, light->opts.linearAttn, light->opts.expAttn, 0.0f);
				fastfrag.lightOrigin = { e->origin,1.0f };
				e->highFreqFrag = renderSystem.vertexCache->AllocTransientUniform(&fastfrag, sizeof(fastfrag));
				e->highFreqVert = renderSystem.vertexCache->AllocTransientUniform(&fastvert, sizeof(fastvert));
			}
			return;
		}

		if (node->GetEntity().GetType() != ENT_MODEL) return;

		glm::mat4 worldMatrix = node->GetLocalToWorldMatrix();
		// W-V-P -> P-V-W
		auto modelViewMatrix = viewMatrix * worldMatrix;

		Bounds entityBounds = node->GetEntity().GetModel()->GetBounds().Transform(modelViewMatrix);
		if (view->frustum.Intersects2(entityBounds))
		{
			viewEntity_t* ent = (viewEntity_t*)R_FrameAlloc(sizeof(*ent));
			auto* model = node->GetEntity().GetModel();
			// entity chain
			ent->next = view->viewEntites;
			ent->modelMatrix = worldMatrix;
			ent->modelViewMatrix = modelViewMatrix;
			ent->mvp = view->projectionMatrix * ent->modelViewMatrix;
			view->viewEntites = ent;

			fastvert.localToWorldMatrix = worldMatrix;
			fastvert.WVPMatrix = ent->mvp;
			fastvert.normalMatrix = transpose(inverse(mat3(worldMatrix)));
			ent->highFreqVert = vc.AllocTransientUniform(&fastvert, sizeof(fastvert));

			for (int entSurf = 0; entSurf < model->GetNumSurface(); ++entSurf)
			{
				const auto* surf = model->GetSurface(entSurf);
				const Bounds surfBounds = surf->surf.bounds.Transform(modelViewMatrix);
				if (view->frustum.Intersects2(surfBounds))
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

					for (int stage = 0; stage < STAGE_COUNT; ++stage)
					{
						const stage_t& stageref = surf->shader->GetStage(static_cast<eStageType>(stage));
						if (stageref.enabled)
						{
							uint32 flg_x = (stageref.coverage & FLG_X_COVERAGE_MASK) << FLG_X_COVERAGE_SHIFT;
							fastfrag.alphaCutoff.x = stageref.alphaCutoff;
							fastfrag.matDiffuseFactor = stageref.diffuseScale;
							fastfrag.matEmissiveFactor = stageref.emissiveScale;
							fastfrag.matMRFactor.x = stageref.roughnessScale;
							fastfrag.matMRFactor.y = stageref.metallicScale;
							fastfrag.params.x = uintBitsToFloat(flg_x);
							drawSurf->highFreqFrag[stage] = vc.AllocTransientUniform(&fastfrag, sizeof(fastfrag));
						}
					}

					view->numDrawSurfs++;
				}
			}
		}
	}
}