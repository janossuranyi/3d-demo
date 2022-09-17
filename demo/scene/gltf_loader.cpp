
#include <glm/gtc/type_ptr.hpp>
#include "logger.h"
#include "gltf_loader.h"


void GltfLoader::setEntityTransform(xNode* ent, const tinygltf::Node& node)
{
	if (node.matrix.size())
	{
		ent->applyMatrix(glm::make_mat4(node.matrix.data()));
	}
	else
	{
		if (node.translation.size())
		{
			ent->translation = vec3(glm::make_vec3(node.translation.data()));
		}
		if (node.rotation.size())
		{
			ent->rotation = quat(float(node.rotation[3]), float(node.rotation[0]), float(node.rotation[1]), float(node.rotation[2]));
		}
		if (node.scale.size())
		{
			ent->scale = vec3(glm::make_vec3(node.scale.data()));
		}
		ent->changed = true;
	}
}

bool GltfLoader::load()
{
    //using namespace tinygltf;

    tinygltf::TinyGLTF loader;
    std::string warn, err;
	// try binary first

	if (!loader.LoadBinaryFromFile(&_glmodel, &err, &warn, _filename))
	{
		Error("%s, line %d: %s,%s", __FILE__, __LINE__, err.c_str(), warn.c_str());
		// then ascii
		if (!loader.LoadASCIIFromFile(&_glmodel, &err, &warn, _filename))
		{
			Error("%s, line %d: %s,%s", __FILE__, __LINE__, err.c_str(), warn.c_str());
			return false;
		}
	}

	Info(" Gltf scene statistics");
	Info("******************************************************************");
	Info("Filename:  %s", _filename.c_str());
	Info("#scene     %d", _glmodel.scenes.size());
	Info("#node      %d", _glmodel.nodes.size());
	Info("#mesh      %d", _glmodel.meshes.size());
	Info("#material  %d", _glmodel.materials.size());
	Info("#texture   %d", _glmodel.textures.size());
	Info("#image     %d", _glmodel.images.size());
	Info("#camera    %d", _glmodel.cameras.size());
	Info("#light     %d", _glmodel.lights.size());
	Info("*****************************************************************");

	_glscene = &_glmodel.scenes[_glmodel.defaultScene];

	for (auto x : _glscene->nodes)
	{
		_scene.root.push_back(x);
	}

	for (uint i = 0; i < _glmodel.nodes.size(); ++i)
	{
		uint xnode = _scene.addNode();
		xNode* pnode = _scene.nodes[xnode].get();

		const tinygltf::Node& node = _glmodel.nodes[i];

		Info("Processing node '%s'", node.name.c_str());
		if (node.camera > 0)
		{
			pnode->type = xNode::eType::Camera;
			pnode->value = node.camera;
		}
		else if (node.mesh > 0)
		{
			pnode->type = xNode::eType::Mesh;
			pnode->value = node.mesh;
		}
		else
		{
			for (auto& ext : node.extensions)
			{
				if (ext.first == "KHR_lights_punctual")
				{
					pnode->type = xNode::eType::Light;
					pnode->value = ext.second.GetNumberAsInt();
				}
			}
		}

		setEntityTransform(pnode, node);

	}
    return false;
}
