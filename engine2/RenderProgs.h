#pragma once

#include <string>
#include "./RenderCommon.h"

namespace jsr {

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
		PRG_VERTEX_COLOR
	};

	struct shader_t
	{
		std::string name;
		eShaderStage stage;
		eVertexLayout vertexLayout;

	};

	struct renderProgram_t
	{
		
	};
}