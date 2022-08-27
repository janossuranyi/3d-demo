#pragma once

#include <vector>
#include <memory>
#include "light.h"
#include "camera.h"
#include "mesh.h"
#include "entity3d.h"
#include "material.h"

class World
{
public:
	World();

	Light& createLight(Light::Type type);
	Camera& createCamera(Camera::Type);
	Mesh3D& createMesh3D();
	Entity3D& createEntity(Entity3D::Type type);
	Material& createMaterial();

	bool loadWorld(const std::string& filename);

	Mesh3D& getMesh(int id);
	Light& getLight(int id);
	Entity3D& getEntity(int id);
	Camera& getCamera(int id);
	RenderMesh3D::Ptr getRenderMesh(int id);
private:
	std::vector<Light> m_lights;
	std::vector<Camera> m_cameras;
	std::vector<Mesh3D> m_meshes;
	std::vector<RenderMesh3D::Ptr> m_renderMeshes;
	std::vector<Entity3D> m_entities;
	std::vector<Material> m_materials;
};
