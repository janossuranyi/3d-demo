#pragma once

#include "loader.h"
#include <tiny_gltf.h>

class GltfLoader : public SceneLoader
{
public:
	GltfLoader() = delete;
	GltfLoader(GltfLoader&) = delete;
	GltfLoader(GltfLoader&&) = delete;
	GltfLoader& operator=(GltfLoader&) = delete;
	GltfLoader& operator=(GltfLoader&&) = delete;

	GltfLoader(const std::string& filename, xScene& scene) : SceneLoader(filename, scene) {}

	bool load() override;

private:
	tinygltf::Model _glmodel;
	tinygltf::Scene* _glscene;


	void setEntityTransform(xNode* ent, const tinygltf::Node& node);

};