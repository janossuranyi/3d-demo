#pragma once

#include <string>
#include <array>
#include <memory>
#include <glm/glm.hpp>

#include "./RenderCommon.h"

#define SSAO_KERNEL_SAMPLES 12

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
		UBB_VS_VIEW_PARAMS,
		UBB_VS_DRAW_PARAMS,
		UBB_FS_VIEW_PARAMS,
		UBB_FS_DRAW_PARAMS,
		UBB_LIGHT_DATA,
		UBB_COMMON_DATA,
		UBB_SHARED_DATA
	};

	struct uboCommonData_t
	{
		glm::vec4 renderTargetRes;
		glm::vec4 shadowRes;
		glm::vec4 bloomRes;
		glm::vec4 ssaoKernel[ SSAO_KERNEL_SAMPLES ];
	};

	struct uboSharedData_t
	{
		glm::vec4 params[8];
	};

	struct uboLightData_t
	{
		glm::mat4 projectMatrix;
		glm::vec4 shadowparams;
		glm::vec4 origin;
		glm::vec4 direction;
		glm::vec4 color;
		glm::vec4 attenuation;
		// spotLightParams
		// x = spotCosCutoff
		// y = spotCosInnerCutoff
		// z = spotExponent
		glm::vec4 params;
	};

	struct uboVSViewParams_t
	{
		glm::mat4 viewMatrix;
		glm::mat4 projectMatrix;
		glm::mat4 invProjectMatrix;
	};
	struct uboVSDrawParams_t 
	{
		glm::mat4 localToWorldMatrix;
		glm::mat4 modelViewMatrix;
		glm::mat4 WVPMatrix;
		glm::mat4 normalMatrix;
	};

	struct uboFSViewParams_t
	{
		glm::mat4 invProjMatrix;
		glm::mat4 projectMatrix;
		glm::vec4 debugparams;
		glm::vec4 screenSize;
		glm::vec4 nearFarClip;
		glm::vec4 params;
		glm::vec4 viewOrigin;
		glm::vec4 ambientColor;
		glm::vec4 bloomParams;
		glm::vec4 bloomParams2;
	};

	struct uboFSDrawParams_t 
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
		SHADER_STAGE_EMPTY = 0,
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
		Properties defs;
	};

	class ShaderStage
	{
	private:
		eShaderStage _stage;
		size_t _size;
		uint8_t* _code;

	public:
		ShaderStage() = default;
		~ShaderStage();
		ShaderStage(eShaderStage stage, const std::string& code);
		ShaderStage(eShaderStage stage, const std::vector<uint8_t>& code);
		ShaderStage(eShaderStage stage, const uint8_t* code, size_t size);
		const uint8_t* GetCode() const;
		size_t GetSize() const;
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
		void SetUniform(eShaderProg program, const char* name, int num, const glm::vec4* v);
		void UpdateCommonUniform();
		void BindCommonUniform();
		void BindUniformBlock(eUboBufferBinding binding, const UniformBuffer& buffer);
		void BindUniformBlock(eUboBufferBinding binding, vertCacheHandle_t handle);

		uboCommonData_t			g_commonData;
	private:
		unsigned int			currentProgram;
		bool					initialized;
		static renderProgram_t	builtins[PRG_COUNT];
		vertCacheHandle_t		g_commonData_h;
		bool CreateBuiltinProgram(renderProgram_t& p);
	};

}