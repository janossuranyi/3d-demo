#pragma once

#include "common.h"
#include "node.h"
#include "material.h"
#include "light.h"
#include "camera.h"
#include "primitive.h"
#include "mesh.h"
#include "texture.h"
#include "animation.h"

struct xScene
{
	
	std::vector<std::unique_ptr<xNode>>		nodes;
	std::vector<std::unique_ptr<xMaterial>>	materials;
	std::vector<std::unique_ptr<xLight>>	lights;
	std::vector<std::unique_ptr<Mesh>>		meshes;
	std::vector<std::unique_ptr<Image>>		images;
	std::vector<Sampler>					samplers;
	std::vector<Texture>					textures;
	std::vector<int>						root;

	std::vector<std::unique_ptr<xCamera>>	cameras;
	std::vector<std::unique_ptr<Animation>> animations;

	uint		addNode();

	uint		addMaterial();

	uint		addLight();

	uint		addMesh();

	uint		addCamera();

	uint		addImage();

	uint		addSampler();

	uint		addTexture();

	uint		addAnimation();

	xScene()
	{
	}
};