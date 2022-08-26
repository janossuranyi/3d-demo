#pragma once

struct Material {
	enum class Type { PBR_METALLIC_ROUGHNESS, PBR_SPECULAR_GLOSSINESS };
	enum class AlphaMode { OPAQUE, MASK, BLEND };

	struct Texture {
		int index;
		int texCoord;
	};

	const char* name;
	Type type;

	union {
		pbrMetallicRoughness_t pbrMetallicRoughness;
		pbrSpecularGlossiness_t pbrSpecularGlossiness;
	};

	AlphaMode alphaMode;
	float alphaCutoff;
	bool doubleSided;
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
		emissiveFactor{ 0.0f,0.0f,0.0f },
		baseColorTexture{ -1,-1 },
		metallicRoughnessTexture{ -1,-1 },
		normalTexture{ -1,-1 },
		emissiveTexture{ -1,-1 },
		occlusionTexture{ -1,-1 }
	{}
		
	float baseColorFactor[4];
	float metallicFactor;
	float roughnessFactor;
	float emissiveFactor[3];
	Material::Texture baseColorTexture;
	Material::Texture metallicRoughnessTexture;
	Material::Texture normalTexture;
	Material::Texture emissiveTexture;
	Material::Texture occlusionTexture;

};

struct pbrSpecularGlossiness_t {
	pbrSpecularGlossiness_t() :
		diffuseFactor{ 1.0f,1.0f,1.0f,1.0f },
		specularFactor{ 1.0f,1.0f,1.0f },
		emissiveFactor{ 1.0f,1.0f,1.0f },
		glossinessFactor(0.0f),
		diffuseTexture{ -1,-1 },
		specularGlossinessTexture{ -1,-1 },
		normalTexture{ -1,-1 },
		emissiveTexture{ -1,-1 },
		occlusionTexture{ -1,-1 }
	{}

	float diffuseFactor[4];
	float specularFactor[3];
	float emissiveFactor[3];
	float glossinessFactor;
	Material::Texture diffuseTexture;
	Material::Texture specularGlossinessTexture;
	Material::Texture normalTexture;
	Material::Texture emissiveTexture;
	Material::Texture occlusionTexture;
};
