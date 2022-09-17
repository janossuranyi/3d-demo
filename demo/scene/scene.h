#pragma once

#include "common.h"
#include "node.h"
#include "material.h"
#include "light.h"
#include "camera.h"
#include "primitive.h"
#include "mesh.h"

struct Scene
{
	
	std::vector<std::unique_ptr<Node>>		nodes;
	std::vector<std::unique_ptr<Material>>	materials;
	std::vector<std::unique_ptr<Light>>		lights;
	std::vector<std::unique_ptr<Primitive>>	primitives;
	std::vector<std::unique_ptr<Mesh>>		meshes;

	std::unique_ptr<Camera>					camera;

	uint		addNode();

	uint		addMaterial();

	uint		addLight();

	uint		addPrimitive();

	uint		addMesh();

	Scene()
	{
		camera.reset(new Camera());
	}
};