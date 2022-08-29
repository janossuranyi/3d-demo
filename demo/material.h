#pragma once
#include <string>

struct TextureInfo {
	int index;
	int texCoord;
};

/*
* 
* 
* The base color texture MUST contain 8-bit values encoded with the sRGB opto-electronic transfer function so RGB values 
* MUST be decoded to real linear values before they are used for any computations. 
* To achieve correct filtering, the transfer function SHOULD be decoded before performing linear interpolation.
* 
* The textures for metalness and roughness properties are packed together in a single texture called metallicRoughnessTexture.
* Its green channel contains roughness values and its blue channel contains metalness values.
* This texture MUST be encoded with linear transfer function and MAY use more than 8 bits per channel.
*/
struct pbrMetallicRoughness_t {

	pbrMetallicRoughness_t() :
		baseColorFactor{ 1.0f,1.0f,1.0f,1.0f },
		metallicFactor(0.0f),
		roughnessFactor(1.0f),
		baseColorTexture{ -1,-1 },
		metallicRoughnessTexture{ -1,-1 }
	{}
		
	float baseColorFactor[4];
	float metallicFactor;
	float roughnessFactor;
	TextureInfo baseColorTexture;
	TextureInfo metallicRoughnessTexture;
};

struct pbrSpecularGlossiness_t {
	pbrSpecularGlossiness_t() :
		diffuseFactor{ 1.0f,1.0f,1.0f,1.0f },
		specularFactor{ 1.0f,1.0f,1.0f },
		glossinessFactor(0.0f),
		diffuseTexture{ -1,-1 },
		specularGlossinessTexture{ -1,-1 }
	{}

	float diffuseFactor[4];
	float specularFactor[3];
	float glossinessFactor;
	TextureInfo diffuseTexture;
	TextureInfo specularGlossinessTexture;
};

struct Material {
	enum Type { PBR_METALLIC_ROUGHNESS, PBR_SPECULAR_GLOSSINESS };
	enum AlphaMode { ALPHA_MODE_OPAQUE, ALPHA_MODE_MASK, ALPHA_MODE_BLEND };

	Material() :
		id(-1),
		name("noname"),
		type(PBR_METALLIC_ROUGHNESS),
		alphaMode(ALPHA_MODE_OPAQUE),
		emissiveFactor{ 1.0f,1.0f,1.0f },
		emissiveTexture{ -1,-1 },
		normalTexture{ -1,-1 },
		occlusionTexture{ -1,-1 },
		alphaCutoff(0.0f),
		pbrMetallicRoughness(),
		doubleSided(false) {}

	std::string name;
	Type type;
	int id;

	union {
		pbrMetallicRoughness_t pbrMetallicRoughness;
		pbrSpecularGlossiness_t pbrSpecularGlossiness;
	};

	AlphaMode alphaMode;
	float alphaCutoff;
	bool doubleSided;
	float emissiveFactor[3];
	TextureInfo normalTexture;
	TextureInfo emissiveTexture;
	TextureInfo occlusionTexture;

};

