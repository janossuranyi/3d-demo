#ifndef JSE_MATERIAL_H
#define JSE_MATERIAL_H

struct JseMaterial {
	JseRenderState rState;
	JseAlphaMode alphaMode{ JseAlphaMode::Opaque };
	bool doubleSided{ false };
	float alphaCutoff{ 0.5f };
	vec3 normalScale{ 1.0f };
	vec2 uvScale{ 1.0f };
	
	JseString vertexShader;
	JseString fragmentShader;
	JseHashMap<JseString, float> custom_params;
	JseGrapicsPipelineID pipeline;
	JseArray<JseImageID, 8> texture_units;
};

#endif // !JSE_MATERIAL_H
