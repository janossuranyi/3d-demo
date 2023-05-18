#pragma once

#include <string>
#include <array>
#include <glm/glm.hpp>

#include "./RenderCommon.h"

namespace jsr {

	const unsigned int INVALID_PROGRAM = 0xFFFF;
	const int SHADER_UNIFORMS_BINDING = 0;

	// must be synchronized with uniforms.inc.glsl
	struct uboUniforms_t
	{
		alignas(16)
		glm::mat4 localToWorldMatrix;
		glm::mat4 worldToViewMatrix;
		glm::mat4 projectionMatrix;
		glm::mat4 WVPMatrix;
		glm::mat4 normalMatrix;
		glm::vec4 viewOrigin;
		glm::vec4 matDiffuseFactor;
		glm::vec4 matMRFactor;
		glm::vec4 alphaCutoff;
		glm::vec4 debugFlags;
		glm::vec4 clipPlanes;
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
		void UpdateUniforms();
		void BindUniforms();
		uboUniforms_t			uniforms;
	private:
		unsigned int			currentProgram;
		bool					initialized;
		vertCacheHandle_t		uniformsCache;
		static renderProgram_t	builtins[PRG_COUNT];

		bool CreateBuiltinProgram(renderProgram_t& p);
	};

}