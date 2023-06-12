#include <tiny_gltf.h>
#include <memory>
#include <filesystem>
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

	bool R_NodeLightFrustumTest(const viewLight_t* v_light, Node3D* node, const viewDef_t* view)
	{
		using namespace glm;

		// shadow matrix
		auto const modelMtx = node->GetLocalToWorldMatrix();
		auto const lightPos = vec3(v_light->origin); // VS!!!
		auto const lightView = lookAt(lightPos, lightPos + vec3(v_light->axis), { 0.0f,1.0f,0.0f });
		auto const modelView = lightView * view->renderView.viewMatrix * modelMtx;
		auto const bounds = node->GetEntity().GetModel()->GetBounds().Transform(modelView);

		return v_light->frustum.Intersects2(bounds);
	}

	void RenderWorld::AddShadowOnlyEntities(viewLight_t* light, viewDef_t* view)
	{
		for (int i = 0; i < rootnodes.size(); ++i)
		{
			Node3D* node = nodes[i];
			if (node->GetEntity().IsLight() || node->viewCount == renderSystem.viewCount)
			{
				continue;
			}
			int numChildren = node->GetNumChildren();
			Node3D** children = node->GetChildren();
			for (int k = 0; k < numChildren; ++k)
			{
				if (R_NodeLightFrustumTest(light, children[k], view))
				{
					RenderNode(node, view, true);
				}
			}
			if (R_NodeLightFrustumTest(light, node,view))
			{
				RenderNode(node, view, true);
			}
		}

	}

	void RenderWorld::RenderView(viewDef_t* view)
	{
		using namespace glm;

		for (int i = 0; i < rootnodes.size(); ++i)
		{
			Node3D* node = nodes[i];
			if (node->GetEntity().IsLight())
			{
				continue;
			}

			int numChildren = node->GetNumChildren();
			Node3D** children = node->GetChildren();
			for (int k = 0; k < numChildren; ++k)
			{
				RenderNode(children[k], view);
			}
			RenderNode(node, view);
		}
		//Info("visibe surface count: %d", view->numDrawSurfs);

		for (int i = 0; i < rootnodes.size(); ++i)
		{
			Node3D* node = nodes[i];
			if (!node->GetEntity().IsLight())
			{
				continue;
			}
			// process light
			RenderLightNode(node, view);
		}

		for (auto* light = view->viewLights; light != nullptr; light = light->next)
		{
			if (light->type != LIGHT_SPOT) continue;

			AddShadowOnlyEntities(light, view);
		}

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

			auto it = gmat.extensions.find("KHR_materials_emissive_strength");
			if (it != gmat.extensions.end())
			{
				auto emissiveStrength = it->second.Get("emissiveStrength").GetNumberAsDouble();
				stage.emissiveScale.w = (float)emissiveStrength;
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
		using namespace glm;
		using namespace std;

		if (gltf_state == nullptr) return;
		if (!nodes.empty())
		{
			nodes.clear();
			rootnodes.clear();
		}

		Model* map = &gltf_state->map;
		vector<int> gltfNodeToLocal;
		gltfNodeToLocal.resize(map->nodes.size());

		for (int i = 0; i < map->nodes.size(); ++i)
		{
			const auto& gnode = map->nodes[i];

			Node3D* node = new Node3D();
			nodes.push_back(node);
			gltfNodeToLocal[i] = (int)nodes.size() - 1;
			node->SetName(gnode.name);

			if ( ! gnode.matrix.empty() )
			{
				vec3 scale{}, translation{}, skew{};
				vec4 persp{};
				quat orient{};
				vector<float> m4(16);
				for ( int i = 0; i < 16; ++i ) m4[ i ] = (float)gnode.matrix[ i ];

				auto m = make_mat4( m4.data() );
				if ( decompose( m, scale, orient, translation, skew, persp ) )
				{
					node->SetOrigin(translation);
					node->SetDir(orient);
					node->SetScale(scale);
				}
			}
			else
			{
				if ( ! gnode.translation.empty() ) {
					node->SetOrigin( make_vec3 ( &gnode.translation[0] ) );
				}
				if ( ! gnode.scale.empty() ) {
					node->SetScale( make_vec3( &gnode.scale[0] ) );
				}
				if ( ! gnode.rotation.empty() ) {
					node->SetDir( quat( (float)gnode.rotation[3], (float)gnode.rotation[0], (float)gnode.rotation[1], (float)gnode.rotation[2] ) );
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
				auto idx = it->second.Get("light").GetNumberAsInt();
				
				auto* light = lights[ gltf_state->map_light_idx[ idx ] ];

				node->GetEntity().SetValue( light );

				light->SetNode( node );
				if ( light->GetType() == LIGHT_POINT )
				{
					node->SetScale( vec3( light->opts.range ) );
				}
				else if ( light->GetType() == LIGHT_SPOT )
				{
					auto const zScale = light->opts.range;
					auto const xyScale = 2.0f * zScale * tan(light->opts.outerConeAngle);
					node->SetScale( vec3( xyScale, xyScale, zScale ) );

					auto angular = eulerAngles(node->GetDir());
					//node->SetDir(angular.x + 90.0f,angular.y,angular.z);

					Info("Spot dir: %f,%f,%f", degrees(angular.x), degrees(angular.y), degrees(angular.z));

				}
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
			light->opts.color = lightColor_t{ glm::make_vec3(e.color.data()), watts};
			light->opts.expAttn = renderGlobals.defaultExpAttn;
			light->opts.linearAttn = renderGlobals.defaultLinearAttn;
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

	void RenderWorld::RenderNode(Node3D* node, viewDef_t* view, bool shadowOnly)
	{
		using namespace glm;

		if (node->GetEntity().GetType() != ENT_MODEL) return;

		const mat4& viewMatrix = view->renderView.viewMatrix;
		VertexCache& vc = *renderSystem.vertexCache;

		alignas(16) uboFreqHighVert_t fastvert {};
		alignas(16) uboFreqHighFrag_t fastfrag {};

		mat4 worldMatrix = node->GetLocalToWorldMatrix();
		// W-V-P -> P-V-W
		auto modelViewMatrix = viewMatrix * worldMatrix;

		Bounds entityBounds = node->GetEntity().GetModel()->GetBounds().Transform(modelViewMatrix);
		if (shadowOnly || view->frustum.Intersects2(entityBounds))
		{			
			node->viewCount = renderSystem.viewCount;
			viewEntity_t* ent = (viewEntity_t*)R_FrameAlloc(sizeof(*ent));
			auto* model = node->GetEntity().GetModel();
			// entity chain
			ent->shadowOnly = shadowOnly;
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
				//const Bounds surfBounds = surf->surf.bounds.Transform(modelViewMatrix);
				//if (view->frustum.Intersects2(surfBounds))
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
							memset(&fastfrag, 0, sizeof(fastfrag));
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
	void RenderWorld::RenderLightNode(Node3D* node, viewDef_t* view)
	{
		using namespace glm;


		if (!node->GetEntity().IsLight())
		{
			return;
		}
		alignas(16) uboFreqHighVert_t fastvert {};
		alignas(16) uboLightData_t ubolight {};

		const mat4& viewMatrix = view->renderView.viewMatrix;
		VertexCache& vc = *renderSystem.vertexCache;

		auto const* light = node->GetEntity().GetLight();
		auto const worldMatrix = node->GetLocalToWorldMatrix();
		auto const origin = worldMatrix[3];
		auto const modelViewMatrix = viewMatrix * worldMatrix;
		auto const lightBounds = Bounds(-vec3(light->opts.range / 2.0f), vec3(light->opts.range / 2.0f)).Transform(modelViewMatrix);
		if (view->frustum.Intersects2(lightBounds))
		{
			auto lightDir = (node->GetDir() * vec4(0.0f, 0.0f, -1.0f, 0.0f));
			viewLight_t* e = (viewLight_t*)R_FrameAlloc(sizeof(*e));
			e->type = light->GetType();
			e->next = view->viewLights;
			e->origin = viewMatrix * origin;
			e->axis = viewMatrix * lightDir;
			e->range = light->opts.range;
			e->shader = light->GetShader();
			e->color = light->opts.color.color;
			e->coneAngle = light->opts.outerConeAngle;
			e->remove = false;
			view->viewLights = e;

			fastvert.WVPMatrix = view->projectionMatrix * modelViewMatrix;
			fastvert.localToWorldMatrix = worldMatrix;
			ubolight.lightColor = e->color;
			ubolight.lightAttenuation = vec4(e->range, light->opts.linearAttn, light->opts.expAttn, 0.0f);
			ubolight.lightOrigin = { e->origin,1.0f };
			if (light->GetType() == LIGHT_SPOT)
			{
				ubolight.spotLightParams.w = 1.0f;
				ubolight.spotLightParams.x = cos(light->opts.outerConeAngle);
				ubolight.spotLightParams.y = cos(light->opts.innerConeAngle);
				ubolight.spotLightParams.z = 5.0f;
				ubolight.spotDirection = { e->axis,0.0f };
				// shadow matrix
				auto const lightPos = vec3(e->origin);
				mat4 const lightProj = perspective(light->opts.outerConeAngle * 2.0f, 1.0f, view->nearClipDistance, view->farClipDistance);
				mat4 const lightView = lookAt(lightPos, lightPos + vec3(e->axis), { 0.0f,1.0f,0.0f });

				e->frustum = Frustum(lightProj);
				ubolight.lightProjMatrix = lightProj * lightView;
				ubolight.shadowparams.x = 1.0f / renderGlobals.shadowResolution;
				ubolight.shadowparams.y = renderGlobals.shadowScale;
				ubolight.shadowparams.z = renderGlobals.shadowBias;
			}
			else
			{
				ubolight.shadowparams.y = 0.0f;
			}

			e->highFreqVert = vc.AllocTransientUniform(&fastvert, sizeof(fastvert));
			e->lightData = vc.AllocTransientUniform(&ubolight, sizeof(ubolight));
		}
	}


}