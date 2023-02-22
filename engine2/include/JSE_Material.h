#ifndef JSE_MATERIAL_H
#define JSE_MATERIAL_H

enum class JseTexBinding {
	BASE_COLOR,	// RGB(A)
	NORMAL_MAP,	// X,Y,Z,H
	PBR_PARAMS,	// R=Occlusion/SpecularR, B=Metalness/SpecualrB, G=Roughness/SpecularG, A=Spec-Occlusion
	EMISSIVE,
	SKYBOX,
	DIFFUSE_IRRADIANCE,
	SPECULAR_IBL,
	BRDF_INTEGRATION,
	MISC0,
	MISC1,
	MISC2,
	MISC3,
	MISC4,
	MISC5,
	MISC6,
	MISC7,
	COUNT
};

struct JseMaterial {
	JseString name;
	JseRenderState rState;
	JseAlphaMode alphaMode{ JseAlphaMode::Opaque };
	bool doubleSided{ false };
	float alphaCutoff{ 0.5f };
	float normalScale{ 1.0f };
	float aoStrength{ 1.0f };
	float metallicFactor{ 1.0f };
	float roughnessFactor{ 1.0f };
	vec2 uvScale{ 1.0f };
	vec4 baseColorFactor{ 1.0f };
	vec3 emissiveFactor{ 0.0f };
	JseGrapicsPipelineID pipeline;
	JseArray<JseImageID, static_cast<size_t>(JseTexBinding::COUNT)> texture_units;
	JseHashMap<JseString, JseString> defines;

	JseString vertexShader;
	JseString fragmentShader;
	JseString geometryShader;
	JseString baseColorTex;
	JseString normalTex;
	JseString occlusionTex;
	JseString emissiveTex;
	JseString pbrTex;
};

#endif // !JSE_MATERIAL_H
