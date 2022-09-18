#include "scene.h"

uint xScene::addNode()
{
    nodes.emplace_back();
    nodes.back().reset(new xNode());
    nodes.back()->scene = this;

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

uint xScene::addImage()
{
    images.emplace_back();
    images.back().reset(new Image());

    return images.size() - 1;
}

uint xScene::addSampler()
{
    samplers.emplace_back();

    return samplers.size() - 1;
}

uint xScene::addTexture()
{
    textures.emplace_back();

    return textures.size() - 1;
}

uint xScene::addAnimation()
{
    animations.emplace_back();
    animations.back().reset(new Animation());
    
    return animations.size() - 1;
}
