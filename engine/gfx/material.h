#ifndef GFX_MATERIAL_H
#define GFX_MATERIAL_H

#include <variant>
#include <string>
#include "gfx/renderer.h"

namespace gfx {

	enum MaterialType : uint {
		Material_StandardPBR,
		Material_None
	};

	struct Material {
		MaterialType type;
	};

	// metallic roughness workflow
	struct Material_StandardPBR
	{
		MaterialType type;
		int id;
		AlphaMode alphaMode;
		bool doubleSided;

		/// <summary>
		/// 0: baseColorTexture
		/// 1: normal
		/// 2: metallicRoughnessTexture
		/// 3: emission
		/// </summary>
		Array<TextureBinding, 4> textures;
		const char* sampler_names[4];
		/// <summary>
		/// [0]: r=alphaCutoff g=roughnessFactor, b=metallicFactor, baseColorFactor
		/// </summary>
		union {
			struct {
				float alphaCutoff;
				float roughnessFactor;
				float metallicFactor;
				float pad0_;
				vec4 baseColorFactor;
			};
			vec4 _v[2];
		} params;

		bool valid() const;
		inline String shaderId() const { return "PBR_MetallicRoughness"; }
	};

	//using Material = std::variant<MRMaterial, SGMaterial>;

}

#endif // !GFX_MATERIAL_H
