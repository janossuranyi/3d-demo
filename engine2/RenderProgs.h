#pragma once

#include <string>
#include <array>
#include <glm/glm.hpp>

#include "./RenderCommon.h"

namespace jsr {

	const unsigned int INVALID_PROGRAM = 0xFFFF;

	struct uboUniforms_t
	{
		alignas(16)
		glm::mat4 localToWorldMatrix;
		glm::mat4 worldToViewMatrix;
		glm::mat4 worldToviewProjectionMatrix;
		glm::vec4 viewOrigin;
	};

	enum eVertexLayout
	{
		LAYOUT_NONE = -1,
		LAYOUT_DRAW_VERT,
		LAYOUT_POSITION_ONLY
	};

	enum eShaderStage
	{
		SHADER_STAGE_VERTEX = 1,
		SHADER_STAGE_FRAGMENT = 2,
		SHADER_STAGE_GEOMETRY = 4,
		SHADER_STAGE_COMPUTE = 8,
		SHADER_STAGE_DEFAULT = SHADER_STAGE_VERTEX | SHADER_STAGE_FRAGMENT
	};

	enum eBuiltinProgram
	{
		PRG_VERTEX_COLOR,
//		PRG_ZPASS,
//		PRG_METALLIC_ROUGH_AO,
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
		void UseProgram(eBuiltinProgram program);
		void UpdateUniforms();
		void BindUniforms();
	private:
		unsigned int			currentProgram;
		bool					initialized;
		uboUniforms_t			uniforms;
		vertCacheHandle_t		uniformsCache;
		static renderProgram_t	builtins[PRG_COUNT];

		bool CreateBuiltinProgram(renderProgram_t& p);
	};

}