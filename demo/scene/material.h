#pragma once

#include "common.h"

struct xMaterial
{
	enum class eAlphaMode : uint { OPAQUE, MASK, BLEND };

	glm::vec4		baseColorFactor;
	float			metallicFactor;
	float			roughnessFactor;
	int				baseColorTexture;
	int				metallicRoughnessTexture;
	int				normalTexture;
	float			normalScale;
	int				occlusionTexture;
	float			occlusionStrength;
	int				emissiveTexture;
	glm::vec3		emissiveFactor;
	eAlphaMode		alphaMode;
	float			alphaCutoff;
	bool			doubleSided;

	// Specular-Glossiness
	bool			specularGlossiness;
	glm::vec4		diffuseFactor;
	glm::vec3		specularFactor;
	float			glossinessFactor;
	int				diffuseTexture;
	int				specularGlossinessTexture;

	float			friction;
	int				sound;
	std::string		name;

	xMaterial() :
		baseColorFactor(1.0f),
		metallicFactor(1.0f),
		roughnessFactor(1.0f),
		baseColorTexture(-1),
		metallicRoughnessTexture(-1),
		normalTexture(-1),
		normalScale(1.0f),
		occlusionTexture(-1),
		occlusionStrength(1.0f),
		emissiveTexture(-1),
		emissiveFactor(0.0f),
		alphaMode(eAlphaMode::OPAQUE),
		alphaCutoff(0.5f),
		doubleSided(false),
		specularGlossiness(false),
		diffuseFactor(1.0f),
		specularFactor(1.0f),
		glossinessFactor(1.0f),
		diffuseTexture(-1),
		specularGlossinessTexture(-1),
		friction(0.0f),
		sound(-1)
	{}
};