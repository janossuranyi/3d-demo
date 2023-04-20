#include <GL/glew.h>
#include <string>

#include "RenderProgs.h"
#include "RenderSystem.h"
#include "RenderBackend_GL.h"
#include "Resources.h"

namespace jsr {

	static GLuint R_CreateShader(GLenum stage, const char* name)
	{
		std::string r_name{ name };

		switch (stage)
		{
		case GL_VERTEX_SHADER:
			r_name.append("_vert.glsl");
			break;
		case GL_FRAGMENT_SHADER:
			r_name.append("_frag.glsl");
			break;
		case GL_GEOMETRY_SHADER:
			r_name.append("_geom.glsl");
			break;
		case GL_COMPUTE_SHADER:
			r_name.append("_comp.glsl");
			break;
		}

		GLuint modul = glCreateShader(stage);
		auto source = resourceMgr->GetShaderSource(r_name);
		const char* pStr = source.c_str();
		GL_CHECK(glShaderSource(modul, 1, &pStr, nullptr));

		return modul;
	}

	bool ProgramManager::LowLevelInit()
	{
		for (auto& p : builtins)
		{
			assert(CreateBuiltinProgram(p));
		}
		return true;
	}
	bool ProgramManager::CreateBuiltinProgram(renderProgram_t& p)
	{
		
		return false;
	}
}