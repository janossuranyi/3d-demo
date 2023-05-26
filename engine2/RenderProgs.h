#pragma once

#include <string>
#include <array>
#include <glm/glm.hpp>

#include "./RenderCommon.h"

namespace jsr {

	const unsigned int INVALID_PROGRAM = 0xFFFF;
	const int SHADER_UNIFORMS_BINDING = 4;

#define gShadowBias shadowparams.z
#define gShadowScale shadowparams.y
#define gOneOverShadowRes shadowparams.x
#define gExposure params.y
#define gFlagsX params.x
#define gSpotCosCutoff spotLightParams.x
#define gSpotCosInnerCutoff spotLightParams.y
#define gSpotExponent spotLightParams.z
#define gSpotLight spotLightParams.w
#define gRoughnessFactor matMRFactor.x
#define gMetallicFactor matMRFactor.y
#define gConstantAttnFactor lightAttenuation.x
#define gLinearAttnFactor lightAttenuation.y
#define gQuadraticAttnFactor lightAttenuation.z

	// must be synchronized with uniforms.inc.glsl
	struct uboUniforms_t
	{
		alignas(16)
		glm::mat4 localToWorldMatrix;
		glm::mat4 WVPMatrix;
		glm::mat4 normalMatrix;
		glm::mat4 lightProjMatrix;
		glm::vec4 viewOrigin;
		glm::vec4 matDiffuseFactor;
		glm::vec4 matMRFactor;
		glm::vec4 alphaCutoff;
		glm::vec4 debugFlags;
		glm::vec4 nearFarClip;
		// params.x = FLAG_X_ *
		// params.y = exposure
		// params.z = 1/Shadow resolution
		glm::vec4 params;
		glm::vec4 shadowparams;
		glm::vec4 lightOrig;
		glm::vec4 lightColor;
		glm::vec4 lightAttenuation;
		// spotLightParams
		// x = spotCosCutoff
		// y = spotCosInnerCutoff
		// z = spotExponent
		glm::vec4 spotLightParams;
		glm::vec4 spotDirection;
	};

	enum eUboBufferBinding
	{
		UBB_FREQ_LOW_VERT,
		UBB_FREQ_HIGH_VERT,
		UBB_FREQ_LOW_FRAG,
		UBB_FREQ_HIGH_FRAG
	};

	struct uboFreqLowVert_t
	{
		glm::mat4 viewMatrix;
		glm::mat4 projectMatrix;
		glm::mat4 lightProjMatrix;
	};
	struct uboFreqHighVert_t 
	{
		glm::mat4 localToWorldMatrix;
		glm::mat4 WVPMatrix;
		glm::mat4 normalMatrix;
	};

	struct uboFreqLowFrag_t
	{
		glm::vec4 debugparams;
		glm::vec4 shadowparams;
		glm::vec4 screenSize;
		glm::vec4 oneOverScreenSize;
		glm::vec4 nearFarClip;
		glm::vec4 params;
		glm::vec4 viewOrigin;
		glm::vec4 lightOrig;
		glm::vec4 lightColor;
		glm::vec4 lightAttenuation;
		// spotLightParams
		// x = spotCosCutoff
		// y = spotCosInnerCutoff
		// z = spotExponent
		glm::vec4 spotLightParams;
		glm::vec4 spotDirection;

	};

	struct uboFreqHighFrag_t 
	{
		glm::vec4 matDiffuseFactor;
		glm::vec4 matMRFactor;
		glm::vec4 alphaCutoff;
		glm::vec4 params;
	};

	enum eVertexLayout
	{
		LAYOUT_DRAW_VERT,
		LAYOUT_POSITION_ONLY,
		LAYOUT_COUNT
	};

	enum eShaderStage
	{
		SHADER_STAGE_VERTEX = 1,
		SHADER_STAGE_FRAGMENT = 2,
		SHADER_STAGE_GEOMETRY = 4,
		SHADER_STAGE_COMPUTE = 8,
		SHADER_STAGE_DEFAULT = SHADER_STAGE_VERTEX | SHADER_STAGE_FRAGMENT
	};

	enum eShaderProg
	{
		PRG_TEXTURED,
		PRG_ZPASS,
		PRG_EQUIRECT_TEXTURE,
		PRG_DEFERRED_GBUFFER_MR,
		PRG_COUNT
	};

	struct renderProgram_t
	{
		const char* name;
		unsigned int stages;
		eVertexLayout vertexLayout;
		unsigned int prg;
	};

	class ProgramManager
	{
	public:
		ProgramManager();
		~ProgramManager();
		bool Init();
		void Shutdown();
		bool LowLevelInit();
		bool IsInitialized() const { return initialized; }
		void UseProgram(eShaderProg program);
		void UniformChanged(eUboBufferBinding b);
		void UpdateUniforms();
		void BindUniforms();
		uboUniforms_t			uniforms;

		uboFreqLowVert_t		g_freqLowVert;
		uboFreqHighVert_t		g_freqHighVert;
		uboFreqLowFrag_t		g_freqLowFrag;
		uboFreqHighFrag_t		g_freqHighFrag;

	private:
		unsigned int			currentProgram;
		bool					initialized;
		vertCacheHandle_t		uniformsCache;

		vertCacheHandle_t		c_freqLowVert;
		vertCacheHandle_t		c_freqHighVert;
		vertCacheHandle_t		c_freqLowFrag;
		vertCacheHandle_t		c_freqHighFrag;

		unsigned int			uboChangedBits;

		static renderProgram_t	builtins[PRG_COUNT];

		bool CreateBuiltinProgram(renderProgram_t& p);
	};

}