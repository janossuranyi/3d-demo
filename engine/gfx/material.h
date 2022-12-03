#ifndef GFX_MATERIAL_H
#define GFX_MATERIAL_H

#include "gfx/renderer.h"

namespace gfx {

	struct Material
	{
		String id;
		MaterialType type;
	};

	struct Material_PBR_MetallicRoughness
	{
		String id;
		MaterialType type;

		AlphaMode alphaMode;
		bool doubleSided;

		ProgramHandle shader;

		/// <summary>
		/// 0: baseColorTexture
		/// 1: normal
		/// 2: metallicRoughnessTexture
		/// 3: occlusion
		/// 4: emission
		/// </summary>
		Array<TextureBinding, 5> textures;

		/// <summary>
		/// [0]: r=alphaCutoff g=roughnessFactor, b=metallicFactor, 
		/// [1]: baseColorFactor
		/// </summary>
		Array<vec4, 2> params;
	};

	
}

#endif // !GFX_MATERIAL_H
