#pragma once

#include <vector>
#include <memory>
#include <tiny_gltf.h>
#include "light.h"
#include "camera.h"
#include "mesh.h"
#include "entity3d.h"
#include "material.h"
#include "gpu_texture.h"

class World
{
public:
	World();

	Light& createLight(Light::Type type);
	Camera& createCamera(Camera::Type);
	Mesh3D& createMesh3D();
	Entity3D& createEntity(Entity3D::Type type);
	Material& createMaterial();
	int createTexture();

	bool loadWorld(const std::string& filename);

	Mesh3D& getMesh(int id);
	Light& getLight(int id);
	Entity3D& getEntity(int id);
	Camera& getCamera(int id);
	RenderMesh3D& getRenderMesh(int id);
	GpuTexture2D& getTexture(int id);
	Material& getMaterial(int id);
	void renderWorld(Pipeline& pipeline);

	const std::vector<int>& root() const;

private:

	void renderWorldRecurs(int node, Pipeline& pipeline);
	void createMeshEntity(tinygltf::Model& model, int node);
	void createTransformEntity(tinygltf::Model& model, int node);
	void createLightEntity(tinygltf::Model& model, int node, int light);
	void createCameraEntity(tinygltf::Model& model, int node);
	void setEntityTransform(Entity3D& ent, const tinygltf::Node& node);
	int createTexture(int texture, tinygltf::Model& model, eTextureFormat format);

	std::vector<Light> m_lights;
	std::vector<Camera> m_cameras;
	std::vector<Mesh3D> m_meshes;
	std::vector<RenderMesh3D> m_renderMeshes;
	std::vector<Entity3D> m_entities;
	std::vector<Material> m_materials;
	std::vector<GpuTexture2D> m_textures;
	std::vector<int> m_root;
};
