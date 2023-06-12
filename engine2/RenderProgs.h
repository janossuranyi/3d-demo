#pragma once

#include <string>
#include <array>
#include <glm/glm.hpp>

#include "./RenderCommon.h"

namespace jsr {

	const unsigned int INVALID_PROGRAM = 0xFFFF;

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

	enum eUboBufferBinding
	{
		UBB_FREQ_LOW_VERT,
		UBB_FREQ_HIGH_VERT,
		UBB_FREQ_LOW_FRAG,
		UBB_FREQ_HIGH_FRAG,
		UBB_LIGHT_DATA
	};

	struct uboLightData_t
	{
		glm::mat4 lightProjMatrix;
		glm::vec4 shadowparams;
		glm::vec4 lightOrigin;
		glm::vec4 lightColor;
		glm::vec4 lightAttenuation;
		// spotLightParams
		// x = spotCosCutoff
		// y = spotCosInnerCutoff
		// z = spotExponent
		glm::vec4 spotLightParams;
		glm::vec4 spotDirection;
	};

	struct uboFreqLowVert_t
	{
		glm::mat4 viewMatrix;
		glm::mat4 projectMatrix;
	};
	struct uboFreqHighVert_t 
	{
		glm::mat4 localToWorldMatrix;
		glm::mat4 WVPMatrix;
		glm::mat4 normalMatrix;
	};

	struct uboFreqLowFrag_t
	{
		glm::mat4 invProjMatrix;
		glm::vec4 debugparams;
		glm::vec4 screenSize;
		glm::vec4 nearFarClip;
		glm::vec4 params;
		glm::vec4 viewOrigin;
		glm::vec4 ambientColor;
	};

	struct uboFreqHighFrag_t 
	{
		glm::vec4 matDiffuseFactor;
		glm::vec4 matMRFactor;
		glm::vec4 matEmissiveFactor;
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
		void SetUniform(eShaderProg program, const char* name, int x);
		void SetUniform(eShaderProg program, const char* name, const glm::vec2& x);
		void BindUniformBlock(eUboBufferBinding binding, const UniformBuffer& buffer);
		void BindUniformBlock(eUboBufferBinding binding, vertCacheHandle_t handle);

	private:
		unsigned int			currentProgram;
		bool					initialized;
		static renderProgram_t	builtins[PRG_COUNT];

		bool CreateBuiltinProgram(renderProgram_t& p);
	};

}