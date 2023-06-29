#ifndef JSR_RENDERWORLD_H
#define JSR_RENDERWORLD_H

#include <vector>
#include <memory>
#include <unordered_set>
#include <tiny_gltf.h>
#include "./Bounds.h"
#include "./RenderCommon.h"
#include "./Light.h"
#include "./TaskExecutor.h"

namespace jsr {

	class Node3D;
	class RenderModel;
	class ModelManager;
	class MaterialManager;
	class VertexCache;
	class ImageManager;
	class Image;
	class Material;

	class RenderWorld
	{
	public:
		RenderWorld();
		~RenderWorld();

		bool LoadMapFromGLTF(const std::string& filename);
		void LoadModelsFromGLTF(const std::string& filename);
		void RenderView(viewDef_t* view);
		void InsertNode(const std::string& name, RenderModel* model, const glm::vec3& pos);
		void DestroyWorld();
		Node3D* GetByName(const std::string& name);
		Bounds GetBounds() const;

		float GetExposure() const;
		void SetExposure(float x);

		float exposure;
	private:
		void CreateImagesGLTF();
		void CreateMaterialsGLTF();
		void CreateNodesGLTF();
		void CreateModelsGLTF();
		void CreateLightsGLTF();
		RenderModel* CreateModelGLTF(int mesh);

		void RenderNode(Node3D* node, viewDef_t* view, bool shadowOnly = false);
		void RenderLightNode(Node3D* node, viewDef_t* view);
		void AddShadowOnlyEntities(viewLight_t* light, viewDef_t* view);

		struct gltf_state_t {
			tinygltf::Model map;
			std::vector<int> map_image_idx;
			std::vector<int> map_mater_idx;
			std::vector<int> map_light_idx;
			std::vector<RenderModel*> map_models;
		} *gltf_state;

		Bounds worldBounds;
		std::vector<Node3D*> rootnodes;
		std::vector<Node3D*> nodes;
		std::vector<Light*> lights;

		std::unordered_set<Material*> materials;
		std::unordered_set<Image*> images;
		std::unordered_set<RenderModel*> models;
		TaskList frustumCullTaskList;
		MaterialManager* materialManager;
		ModelManager*	modelManager;
		ImageManager*	imageManager;
		VertexCache*	vertexCache;
	};
}
#endif // !JSR_RENDERWORLD_H
