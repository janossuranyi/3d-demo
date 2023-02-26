#ifndef JSE_MATERIAL_H
#define JSE_MATERIAL_H

#include <utility>

namespace js {

	using MaterialAttribute = std::variant<ivec4, uvec4, vec4, mat4>;
	using MaterialAttributePair = std::pair<JsString, MaterialAttribute>;

	struct Material {
		JsString								id;
		JsString								shader;
		JseRenderState							renderState;
		JseAlphaMode							alphaMode{ JseAlphaMode::Opaque };
		JseGrapicsPipelineID					pipeline;
		JsArray<JseImageID, 16>					textures;
		JsVector<MaterialAttributePair>			attributes;
		bool									castShadow;

		void AddAttribute(const JsString& name, const MaterialAttribute& value);
		void CopyAttributes(int n, void* dest) const;
		size_t size() const;

		JsString GetUniformBlockDef(int binding, const JsString& name, const JsString& instanceName) const;
	};

}
#endif // !JSE_MATERIAL_H
