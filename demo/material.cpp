#include "material.h"

bool Material::hasNormal() const
{
    return normalTexture.index > -1;
}

bool Material::isMetallicRoughness() const
{
    return type == Type::PBR_METALLIC_ROUGHNESS;
}

bool Material::isSpecularGlossiness() const
{
    return type == Type::PBR_SPECULAR_GLOSSINESS;
}
