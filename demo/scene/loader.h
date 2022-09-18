#pragma once

#include <iostream>
#include "scene.h"

class SceneLoader
{
public:
	SceneLoader() = delete;
	SceneLoader(SceneLoader&) = delete;
	SceneLoader(SceneLoader&&) = delete;
	SceneLoader& operator=(SceneLoader&) = delete;
	SceneLoader& operator=(SceneLoader&&) = delete;

	SceneLoader(const std::string& filename, xScene& scene): _filename(filename), _scene(scene) {}
	virtual bool load() { std::cout << "ScanLoader::load not implemented" << std::endl; return false; };

protected:
	std::string _filename;
	xScene& _scene;
};