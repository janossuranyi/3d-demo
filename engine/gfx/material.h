#ifndef GFX_MATERIAL_H
#define GFX_MATERIAL_H

#include <variant>
#include <string>
#include "gfx/renderer.h"

namespace gfx {

	struct Material {
		MaterialHandle id{};
		MaterialWorkflow workflow{ MaterialWorkflow::MetallicRoughness };
		AlphaMode alphaMode{ AlphaMode::Opaque };
		bool doubleSided{ true };
		String diffuseTexture;
		String normalTexture;
		String bumpTexture;
		String emissionTexture;
		float alphaCutoff{ 0.5f };
		float roughnessScale{ 1.0f };
		float metallicScale{ 1.0f };
		vec4 baseColor{ 1.0f };
		vec3 normalScale{ 1.0f };
		vec2 uvScale{ 1.0f };

		bool valid() const;
	};

	inline bool Material::valid() const { return id.isValid(); }
}

#endif // !GFX_MATERIAL_H
