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
	tinygltf::Model		_glmodel;
	tinygltf::Scene*	_glscene;

	void		setNodeTransform(xNode* node, const tinygltf::Node& glnode);
	void		parseNodes();
	void		parseLights();
	void		parseCameras();
	void		parseMaterials();
	void		parseImages();
	void		parseSamplers();
	void		parseTextures();
	void		parseAnimations();
	void		parseMeshes();

	void		arrayToFloatArray(const tinygltf::Accessor& access, std::vector<float>& dst);
};
