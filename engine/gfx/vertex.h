#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/half_float.hpp>
#include "renderer.h"

namespace gfx {

	template<VertexDeclType DeclTy>
	struct VertexDecl {};

	template<>
	struct VertexDecl<VertexDeclType::Draw>
	{
		glm::vec3 position;
		glm::half
	};

}
