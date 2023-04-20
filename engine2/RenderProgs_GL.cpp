#include <GL/glew.h>
#include <string>
#include <vector>

#include "RenderProgs.h"
#include "RenderSystem.h"
#include "RenderBackend_GL.h"
#include "Resources.h"
#include "Logger.h"

namespace jsr {

	static bool R_LinkProgram(GLuint& program, GLuint const* shaders, int numShader)
	{
		GL_CHECK(program = glCreateProgram());
		for (int i = 0; i < numShader; ++i)
		{
			GL_CHECK(glAttachShader(program, shaders[i]));
		}
		GL_CHECK(glLinkProgram(program));
	
		GLint result = GL_FALSE;
		GL_CHECK(glGetProgramiv(program, GL_LINK_STATUS, &result));

		if (result == GL_FALSE)
		{
			GLint infologLen;
			GL_CHECK(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologLen));
			if (infologLen > 0) {
				std::vector<char> logBuf(infologLen);
				GL_CHECK(glGetProgramInfoLog(program, infologLen, nullptr, logBuf.data()));
				Error("Linking of shader program failed: %s", logBuf.data());
			}
			R_DeleteShaders(shaders, numShader);
			GL_CHECK(glDeleteProgram(program));
			program = 0;
			return false;
		}
		return true;
	}

	static void R_DeleteShaders(GLuint const* list, int numShader)
	{
		for (int i = 0; i < numShader; ++i)
		{
			if (glIsShader(list[i])) glDeleteShader(list[i]);
		}
	}
	static bool R_CompileShader(GLuint shader)
	{
		if (!glIsShader(shader)) return false;

		GLint result = GL_FALSE;

		GL_CHECK(glCompileShader(shader));
		GL_CHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &result));

		if (result == GL_FALSE)
		{
			GLint infologLen;
			GL_CHECK(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen));
			if (infologLen > 0) {
				std::vector<char> logBuf(infologLen);
				GL_CHECK(glGetShaderInfoLog(shader, infologLen, nullptr, logBuf.data()));
				Error("Shader compile error: %s", logBuf.data());
			}

			return false;
		}

		return true;
	}

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
		if (!modul) return 0;

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

		std::vector<GLuint> shaders;
		if (p.stages & SHADER_STAGE_VERTEX)		shaders.push_back(R_CreateShader(GL_VERTEX_SHADER, p.name));
		if (p.stages & SHADER_STAGE_FRAGMENT)	shaders.push_back(R_CreateShader(GL_FRAGMENT_SHADER, p.name));
		if (p.stages & SHADER_STAGE_GEOMETRY)	shaders.push_back(R_CreateShader(GL_GEOMETRY_SHADER, p.name));
		if (p.stages & SHADER_STAGE_COMPUTE)	shaders.push_back(R_CreateShader(GL_COMPUTE_SHADER, p.name));

		bool allOk = true;
		for (auto k : shaders)
		{
			if (k == 0) allOk = false;
		}

		if (allOk)
		{
			for (auto k : shaders)
			{
				allOk &= R_CompileShader(k);
			}
		}
		else
		{
			R_DeleteShaders(shaders.data(), shaders.size());
			return false;
		}

		GLuint program;
		allOk = allOk && R_LinkProgram(program, shaders.data(), shaders.size());
		if (allOk)
		{
			// All shader stage are compiled
			p.prg = program;
			R_DeleteShaders(shaders.data(), shaders.size());
		}

		return allOk;
	}
}