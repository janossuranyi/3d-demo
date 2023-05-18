#ifndef JSR_RENDERWORLD_H
#define JSR_RENDERWORLD_H

#include <vector>
#include <memory>
#include <unordered_set>
#include <tiny_gltf.h>
#include "./Bounds.h"
#include "./RenderCommon.h"
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
		void RenderView(viewDef_t* view);
		void DestroyWorld();
		Bounds GetBounds() const;
	private:
		void CreateImagesGLTF();
		void CreateMaterialsGLTF();
		void CreateNodesGLTF();
		RenderModel* CreateModelGLTF(int mesh);

		tinygltf::Model* map;
		std::vector<int> map_image_idx;
		std::vector<int> map_mater_idx;

		Bounds worldBounds;
		std::vector<Node3D*> rootnodes;
		std::vector<Node3D*> nodes;


		std::unordered_set<Material*> materials;
		std::unordered_set<Image*> images;
		std::unordered_set<RenderModel*> models;


		MaterialManager* materialManager;
		ModelManager*	modelManager;
		ImageManager*	imageManager;
		VertexCache*	vertecCache;
	};
}
#endif // !JSR_RENDERWORLD_H
