#ifndef JSE_MATERIAL_H
#define JSE_MATERIAL_H

enum JseTexBinding {
	JSE_TEXBIND_ALBEDO = 0,	// RGB(A)
	JSE_TEXBIND_NORMAL,	// X,Y,Z,H
	JSE_TEXBIND_METALLIC_ROUGHNESS_SPECULAR = 2,	// R=SpecularR, B=Metalness/SpecualrG, G=Roughness/SpecularG, A=AO
	JSE_TEXBIND_EMISSIVE,
	JSE_TEXBIND_SKYBOX_CUBE_MAP,
	JSE_TEXBIND_DIFFUSE_IRRADIANCE,
	JSE_TEXBIND_SPEC_IBL,
	JSE_TEXBIND_BRDF_INTEG,
	JSE_TEXBIND_MISC0,
	JSE_TEXBIND_MISC1,
	JSE_TEXBIND_MISC2,
	JSE_TEXBIND_MISC3,
	JSE_TEXBIND_MISC4,
	JSE_TEXBIND_MISC5,
	JSE_TEXBIND_MISC6,
	JSE_TEXBIND_MISC7,
	JSE_TEXBIND_COUNT
};

struct JseMaterial {
	JseRenderState rState;
	JseAlphaMode alphaMode{ JseAlphaMode::Opaque };
	bool doubleSided{ false };
	float alphaCutoff{ 0.5f };
	vec3 normalScale{ 1.0f };
	vec2 uvScale{ 1.0f };
	
	JseString vertexShader;
	JseString fragmentShader;
	JseGrapicsPipelineID pipeline;
	JseArray<JseImageID, JSE_TEXBIND_COUNT> texture_units;
};

#endif // !JSE_MATERIAL_H
