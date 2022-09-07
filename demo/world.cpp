#include <cassert>
#include <utility>
#include <tiny_gltf.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL2.h>

#include "logger.h"
#include "world.h"
#include "camera.h"
#include "light.h"
#include "mesh.h"
#include "entity3d.h"
#include "material.h"
#include "pipeline.h"

#define _TG tinygltf::

World::World()
{

}

Light& World::createLight(Light::Type type)
{
	Light& res = m_lights.emplace_back(type);
	res.setId(int(m_lights.size()) - 1);

	return res;
}

Camera& World::createCamera(Camera::Type type)
{
	Camera& res = m_cameras.emplace_back(type);
	res.setId(int(m_cameras.size() - 1));

	return res;
}

Mesh3D& World::createMesh3D()
{
	Mesh3D& res			= m_meshes.emplace_back();
	RenderMesh3D& rm	= m_renderMeshes.emplace_back();

	res.setId(int(m_meshes.size() - 1));
	rm.setId(res.id());

	return res;
}

Entity3D& World::createEntity(Entity3D::Type type)
{
	Entity3D& res = m_entities.emplace_back(type, *this);
	res.setId(int(m_entities.size() - 1));

	return res;
}

Material& World::createMaterial()
{
	Material& res = m_materials.emplace_back();
	res.id = int(m_materials.size() - 1);

	return res;
}

int World::createTexture()
{
	GpuTexture2D& tex = m_textures.emplace_back();

	return int(m_textures.size() - 1);
}

bool World::loadWorld(const std::string& filename)
{
	_TG Model model;
	_TG TinyGLTF loader;
	_TG Scene scene;

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

	scene = model.scenes[model.defaultScene];

	for (auto r : scene.nodes)
	{
		m_root.push_back(r);
	}

	for (int n = 0; n < model.nodes.size(); ++n)
	{
		Entity3D::Type entityType = Entity3D::Type::TRANSFORM;
		
		int light = -1;

		const _TG Node& node = model.nodes[n];

		Info("Processing node '%s'", node.name.c_str());

		if (node.camera >= 0)
		{
			entityType = Entity3D::Type::CAMERA;
		}
		else if (node.mesh >= 0)
		{
			entityType = Entity3D::Type::MESH;
		}
		for(auto& ext : node.extensions)
		{
			if (ext.first == "KHR_lights_punctual")
			{
				entityType = Entity3D::Type::LIGHT;
				light = ext.second.GetNumberAsInt();
			}
		}

		switch (entityType)
		{
		case Entity3D::Type::MESH:
			createMeshEntity(model, n);
			break;
		case Entity3D::Type::TRANSFORM:
			createTransformEntity(model, n);
			break;
		case Entity3D::Type::LIGHT:
			createLightEntity(model, n, light);
		}

	}

	for (auto& ent : m_entities)
	{
		ent.updateParentChild();
		ent.updateWorldMatrix();
	}


	for (int i = 0; i < model.materials.size(); ++i)
	{
		_TG Material mat = model.materials[i];
		m_materials.emplace_back();
		Material& m = m_materials[m_materials.size() - 1];
		
		if (mat.extensions.size())
		{
			for (auto ext : mat.extensions)
			{
				if (ext.first == "KHR_materials_pbrSpecularGlossiness")
				{
					m.type = Material::Type::PBR_SPECULAR_GLOSSINESS;
					for (const auto& key : ext.second.Keys())
					{
						auto& val = ext.second.Get(key);
						if (key == "diffuseFactor")
						{
							m.pbrSpecularGlossiness.diffuseFactor[0] = static_cast<float>(val.Get(0).GetNumberAsDouble());
							m.pbrSpecularGlossiness.diffuseFactor[1] = static_cast<float>(val.Get(1).GetNumberAsDouble());
							m.pbrSpecularGlossiness.diffuseFactor[2] = static_cast<float>(val.Get(2).GetNumberAsDouble());
							m.pbrSpecularGlossiness.diffuseFactor[3] = static_cast<float>(val.Get(3).GetNumberAsDouble());
						}
						else if (key == "specularFactor")
						{
							m.pbrSpecularGlossiness.specularFactor[0] = static_cast<float>(val.Get(0).GetNumberAsDouble());
							m.pbrSpecularGlossiness.specularFactor[1] = static_cast<float>(val.Get(1).GetNumberAsDouble());
							m.pbrSpecularGlossiness.specularFactor[2] = static_cast<float>(val.Get(2).GetNumberAsDouble());
						}
						else if (key == "glossinessFactor")
						{
							m.pbrSpecularGlossiness.glossinessFactor = static_cast<float>(val.GetNumberAsDouble());
						}
						else if (key == "diffuseTexture")
						{
							int x = createTexture(val.Get("index").GetNumberAsInt(), model, eTextureFormat::SRGB_A);
							m.pbrSpecularGlossiness.diffuseTexture.index = x;
							m.pbrSpecularGlossiness.diffuseTexture.texCoord = val.Get("texCoord").GetNumberAsInt();
						}
						else if (key == "specularGlossinessTexture")
						{
							int x = createTexture(val.Get("index").GetNumberAsInt(), model, eTextureFormat::SRGB_A);
							m.pbrSpecularGlossiness.specularGlossinessTexture.index = x;
							m.pbrSpecularGlossiness.specularGlossinessTexture.texCoord = val.Get("texCoord").GetNumberAsInt();
						}
					}
				}
			}
		}
		else
		{
			// pbrMetallicRoughness
			m.pbrMetallicRoughness.baseColorFactor = glm::make_vec4(mat.pbrMetallicRoughness.baseColorFactor.data());
			m.pbrMetallicRoughness.metallicFactor = float(mat.pbrMetallicRoughness.metallicFactor);
			m.pbrMetallicRoughness.roughnessFactor = float(mat.pbrMetallicRoughness.roughnessFactor);

			int x = createTexture(mat.pbrMetallicRoughness.baseColorTexture.index, model, eTextureFormat::SRGB_A);
			m.pbrMetallicRoughness.baseColorTexture.index = x;
			m.pbrMetallicRoughness.baseColorTexture.texCoord = mat.pbrMetallicRoughness.baseColorTexture.texCoord;

			x = createTexture(mat.pbrMetallicRoughness.metallicRoughnessTexture.index, model, eTextureFormat::RGBA);
			m.pbrMetallicRoughness.metallicRoughnessTexture.index = x;
			m.pbrMetallicRoughness.metallicRoughnessTexture.texCoord = mat.pbrMetallicRoughness.metallicRoughnessTexture.texCoord;
		}

		m.alphaCutoff				= float(mat.alphaCutoff);
		m.doubleSided				= mat.doubleSided;
		m.name						= mat.name;
		m.alphaMode					= Material::AlphaMode::ALPHA_MODE_OPAQUE;
		m.emissiveFactor			= glm::make_vec3(mat.emissiveFactor.data());
		int x = createTexture(mat.normalTexture.index, model, eTextureFormat::RGBA);
		m.normalTexture.index		= x;
		m.normalTexture.texCoord	= mat.normalTexture.texCoord;
		x = createTexture(mat.emissiveTexture.index, model, eTextureFormat::RGBA);
		m.emissiveTexture.index		= x;
		m.emissiveTexture.texCoord	= mat.emissiveTexture.texCoord;
		x = createTexture(mat.occlusionTexture.index, model, eTextureFormat::RGBA);
		m.occlusionTexture.index	= x;
		m.occlusionTexture.texCoord = mat.occlusionTexture.texCoord;

		if (mat.alphaMode == "MASK")		m.alphaMode = Material::AlphaMode::ALPHA_MODE_MASK;
		else if (mat.alphaMode == "BLEND")	m.alphaMode = Material::AlphaMode::ALPHA_MODE_BLEND;
	}

	return true;

}

Mesh3D& World::getMesh(int id)
{
	assert(m_meshes.size() > id);

	return m_meshes[id];
}

Entity3D& World::getEntity(int id)
{
	assert(m_entities.size() > id);

	return m_entities[id];
}

Camera& World::getCamera(int id)
{
	assert(m_cameras.size() > id);

	return m_cameras[id];
}

RenderMesh3D& World::getRenderMesh(int id)
{
	assert(id < m_renderMeshes.size());

	return m_renderMeshes[id];
}

GpuTexture2D& World::getTexture(int id)
{
	assert(id < m_textures.size());

	return m_textures[id];
}

Material& World::getMaterial(int id)
{
	assert(id < m_materials.size());

	return m_materials[id];
}

void World::renderWorld(Pipeline& pipeline)
{
	for (int n : m_root)
	{
		renderWorldRecurs(n, pipeline);
	}
}

const std::vector<int>& World::root() const
{
	return m_root;
}

void World::renderWorldRecurs(int node_, Pipeline& pipeline)
{
	Entity3D& node = getEntity(node_);

	for (int c : node.children())
	{
		renderWorldRecurs(c, pipeline);
	}

	if (node.type() == Entity3D::Type::MESH)
	{
		RenderMesh3D& rm = getRenderMesh(node.value());
		pipeline.setWorldMatrix(node.worldMatrix());
		pipeline.update();
		rm.render(pipeline, *this);
	}

}

void World::createMeshEntity(tinygltf::Model& model, int n)
{
	const _TG Node& node = model.nodes[n];
	const _TG Mesh& mesh = model.meshes[node.mesh];
	for (auto& prim : mesh.primitives)
	{
		Entity3D& ent = createEntity(Entity3D::Type::MESH);
		Mesh3D& mesh = createMesh3D();

		if (!mesh.importFromGLTF(model, prim)) {
			Error("%s import GLTF primitive error !!!");
		}

		const int meshId = mesh.id();

		m_renderMeshes[meshId].compile(mesh);
		m_renderMeshes[meshId].setMaterial(mesh.material());
		ent.setValue(meshId);

		setEntityTransform(ent, node);

		for (int ch : node.children)
		{
			ent.addChild(ch);
		}
	}
}

void World::createTransformEntity(tinygltf::Model& model, int n)
{
	const _TG Node& node = model.nodes[n];
	Entity3D& ent = createEntity(Entity3D::Type::TRANSFORM);
	setEntityTransform(ent, node);

	ent.setValue(-1);

	for (int ch : node.children)
	{
		ent.addChild(ch);
	}

}

void World::createLightEntity(tinygltf::Model& model, int n, int ln)
{
	const _TG Node& node = model.nodes[n];
	const _TG Light light = model.lights[ln];
	Entity3D& ent = createEntity(Entity3D::Type::LIGHT);
	Light::Type lightType;

	if (light.type == "point")
	{
		lightType = Light::Type::POINT;
	}
	else if (light.type == "spot")
	{
		lightType = Light::Type::SPOT;
	}
	else if (light.type == "directional")
	{
		lightType = Light::Type::DIRECTIONAL;
	}
	else
	{
		assert(false);
	}
		
	Light& L = createLight(lightType);

	setEntityTransform(ent, node);

	ent.setValue(L.id());

	for (int ch : node.children)
	{
		ent.addChild(ch);
	}

	L.color = vec4(glm::make_vec3(light.color.data()), float(light.intensity));
	L.name = light.name;

	if (lightType == Light::Type::SPOT || lightType == Light::Type::POINT)
	{
		L.range = float(light.range);
	}
	if (lightType == Light::Type::SPOT)
	{
		L.spot.innerConeAngle = float(light.spot.innerConeAngle);
		L.spot.outerConeAngle = float(light.spot.outerConeAngle);
	}
}

void World::createCameraEntity(tinygltf::Model& model, int n)
{
	const _TG Node& node = model.nodes[n];
	const _TG Camera cam = model.cameras[node.camera];

	Camera::Type cameraType;
	if (cam.type == "perspective")
	{
		cameraType = Camera::Type::PERSPECTIVE;
	}
	else if (cam.type == "orthographic")
	{
		cameraType = Camera::Type::ORTHOGRAPHIC;
	}
	else
	{
		assert(false);
	}

	Camera& camera = createCamera(cameraType);
	Entity3D& ent = createEntity(Entity3D::Type::CAMERA);

	ent.setValue(camera.id());
	setEntityTransform(ent, node);

	for (int ch : node.children)
	{
		ent.addChild(ch);
	}

	camera.name = cam.name;
	if (camera.type == Camera::Type::PERSPECTIVE)
	{
		camera.perspective.aspectRatio = float(cam.perspective.aspectRatio);
		camera.perspective.yfov = float(cam.perspective.yfov);
		camera.perspective.znear = float(cam.perspective.znear);
		camera.perspective.zfar = float(cam.perspective.zfar);
	}
	else
	{
		camera.ortho.znear = float(cam.orthographic.znear);
		camera.ortho.zfar = float(cam.orthographic.zfar);
		camera.ortho.xmag = float(cam.orthographic.xmag);
		camera.ortho.ymag = float(cam.orthographic.ymag);
	}

}

void World::setEntityTransform(Entity3D& ent, const tinygltf::Node& node)
{
	if (node.matrix.size())
	{
		ent.setWorldMatrix(glm::make_mat4(node.matrix.data()));
	}
	else
	{
		if (node.translation.size())
		{
			ent.translate(vec3(glm::make_vec3(node.translation.data())));
		}
		if (node.rotation.size())
		{
			ent.rotate(quat(float(node.rotation[3]), float(node.rotation[0]), float(node.rotation[1]), float(node.rotation[2])));
		}
		if (node.scale.size())
		{
			ent.scale(vec3(glm::make_vec3(node.scale.data())));
		}
		ent.updatMatrix();
	}
}

int World::createTexture(int texture, tinygltf::Model& model, eTextureFormat format)
{
	if (texture == -1) return -1;

	int texId = createTexture();
	GpuTexture2D& texObj = m_textures[texId];

	const _TG Texture tex = model.textures[texture];
	const _TG Image image = model.images[tex.source];
	const _TG Sampler sampler = model.samplers[tex.sampler];

	const _TG BufferView view = model.bufferViews[image.bufferView];
	const _TG Buffer buffer = model.buffers[view.buffer];

	Info("=============================================================");
	Info("image name      : %s", image.name.c_str());
	Info("image w         : %d", image.width);
	Info("image h         : %d", image.height);
	Info("image bits      : %d", image.bits);
	Info("image component : %d", image.component);
	Info("image pixel_type: %d", image.pixel_type);
	Info("image mimeType  : %s", image.mimeType.c_str());

	ePixelFormat srcFormat = ePixelFormat::RGB;

	if (image.bits == 8)
	{
		switch (image.component)
		{
		case 3: srcFormat = ePixelFormat::RGB;
			break;
		case 4: srcFormat = ePixelFormat::RGBA;
			break;
		}
	}
	else if (image.bits == 16)
	{
		switch (image.component)
		{
		case 3: srcFormat = ePixelFormat::RGB16;
			break;
		case 4: srcFormat = ePixelFormat::RGBA16;
			break;
		}
	}

	eDataType dataType = eDataType::UNSIGNED_BYTE;
	switch (image.pixel_type)
	{
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
		dataType = eDataType::UNSIGNED_BYTE;
		break;
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
		dataType = eDataType::UNSIGNED_SHORT;
		break;
	case TINYGLTF_COMPONENT_TYPE_FLOAT:
		dataType = eDataType::FLOAT;
		break;
	}

	texObj.create(image.width, image.height, 0, format, srcFormat, dataType, image.image.data());
	texObj.withDefaultLinearClampEdge().updateParameters();

	return texId;
}

Light& World::getLight(int id)
{
	assert(m_lights.size() < id);

	return m_lights[id];
}
