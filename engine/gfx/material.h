#ifndef GFX_MATERIAL_H
#define GFX_MATERIAL_H

#include "gfx/renderer.h"

namespace gfx {

	struct Material
	{
		bool transparent{};
		TextureBindings textures;
		ProgramHandle program{};
		vec4 colorFactor{ 1.0f };
	};
}

#endif // !GFX_MATERIAL_H
