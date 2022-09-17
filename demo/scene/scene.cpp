#include "scene.h"

uint Scene::addNode()
{
    nodes.emplace_back();
    nodes.back().reset(new Node());

    return nodes.size() - 1;
}

uint Scene::addMaterial()
{
    materials.emplace_back();
    materials.back().reset(new Material());

    return materials.size() - 1;
}

uint Scene::addLight()
{
    lights.emplace_back();
    lights.back().reset(new Light());

    return lights.size() - 1;
}

uint Scene::addPrimitive()
{
    primitives.emplace_back();
    primitives.back().reset(new Primitive());

    return primitives.size() - 1;
}

uint Scene::addMesh()
{
    meshes.emplace_back();
    meshes.back().reset(new Mesh());

    return meshes.size() - 1;
}
