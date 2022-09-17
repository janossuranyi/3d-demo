#include "scene.h"

uint xScene::addNode()
{
    nodes.emplace_back();
    nodes.back().reset(new xNode());

    return nodes.size() - 1;
}

uint xScene::addMaterial()
{
    materials.emplace_back();
    materials.back().reset(new xMaterial());

    return materials.size() - 1;
}

uint xScene::addLight()
{
    lights.emplace_back();
    lights.back().reset(new xLight());

    return lights.size() - 1;
}

uint xScene::addPrimitive()
{
    primitives.emplace_back();
    primitives.back().reset(new Primitive());

    return primitives.size() - 1;
}

uint xScene::addMesh()
{
    meshes.emplace_back();
    meshes.back().reset(new Mesh());

    return meshes.size() - 1;
}

uint xScene::addCamera()
{
    cameras.emplace_back();
    cameras.back().reset(new xCamera());

    return cameras.size() - 1;
}
