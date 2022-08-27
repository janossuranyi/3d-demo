#include <cassert>
#include <tiny_gltf.h>

#include "logger.h"
#include "world.h"
#include "camera.h"
#include "light.h"
#include "mesh.h"
#include "entity3d.h"
#include "material.h"

World::World()
{

}

Light& World::createLight(Light::Type type)
{
	Light& res = m_lights.emplace_back(type);
	res.setId(m_lights.size() - 1);

	return res;
}

Camera& World::createCamera(Camera::Type type)
{
	Camera& res = m_cameras.emplace_back(type);
	res.setId(m_cameras.size() - 1);

	return res;
}

Mesh3D& World::createMesh3D()
{
	Mesh3D& res = m_meshes.emplace_back();
	RenderMesh3D::Ptr rm = m_renderMeshes.emplace_back();

	res.setId( m_meshes.size() - 1 );
	rm->setId(res.id());

	return res;
}

Entity3D& World::createEntity(Entity3D::Type type)
{
	Entity3D& res = m_entities.emplace_back(type, *this);
	res.setId(m_entities.size() - 1);

	return res;
}

Material& World::createMaterial()
{
	Material& res = m_materials.emplace_back();
	res.id = m_materials.size() - 1;

	return res;
}

bool World::loadWorld(const std::string& filename)
{
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;

	std::string warn, err;

	// try binary first
	if (!loader.LoadBinaryFromFile(&model, &err, &warn, filename))
	{
		Error("%s, line %d: %s,%s", __FILE__, __LINE__, err.c_str(), warn.c_str());
		// then ascii
		if (!loader.LoadASCIIFromFile(&model, &err, &warn, filename))
		{
			Error("%s, line %d: %s,%s", __FILE__, __LINE__, err.c_str(), warn.c_str());
			return false;
		}
	}

	Info(" Gltf scene statistics");
	Info("******************************************************************");
	Info("Filename:  %s", filename.c_str());
	Info("#scene     %d", model.scenes.size());
	Info("#node      %d", model.nodes.size());
	Info("#mesh      %d", model.meshes.size());
	Info("#material  %d", model.materials.size());
	Info("#texture   %d", model.textures.size());
	Info("#image     %d", model.images.size());
	Info("#camera    %d", model.cameras.size());
	Info("#light     %d", model.lights.size());
	Info("*****************************************************************");


	return true;

}

Mesh3D& World::getMesh(int id)
{
	assert(m_meshes.size() < id);

	return m_meshes[id];
}

Entity3D& World::getEntity(int id)
{
	assert(m_entities.size() < id);

	return m_entities[id];
}

Camera& World::getCamera(int id)
{
	assert(m_cameras.size() < id);

	return m_cameras[id];
}

RenderMesh3D::Ptr World::getRenderMesh(int id)
{
	assert(id < m_renderMeshes.size());

	return m_renderMeshes[id];
}

Light& World::getLight(int id)
{
	assert(m_lights.size() < id);

	return m_lights[id];
}
