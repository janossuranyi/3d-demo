#include <GL/glew.h>
#include <string>
#include <vector>

#include "engine2/RenderProgs.h"
#include "engine2/RenderSystem.h"
#include "./RenderBackend_GL.h"
#include "engine2/Resources.h"
#include "engine2/Logger.h"

namespace jsr {

	renderProgram_t ProgramManager::builtins[] = {
		{"default_textured",		SHADER_STAGE_DEFAULT,	LAYOUT_DRAW_VERT,	INVALID_PROGRAM, {} },
		{"depth_only",				SHADER_STAGE_DEFAULT,	LAYOUT_DRAW_VERT,	INVALID_PROGRAM, {} },
		{"texture_equirect",		SHADER_STAGE_DEFAULT,	LAYOUT_DRAW_VERT,	INVALID_PROGRAM, {} },
		{"deferred_gbuffer_mr",		SHADER_STAGE_DEFAULT,	LAYOUT_DRAW_VERT,	INVALID_PROGRAM, {} },
		{"deferred_light",			SHADER_STAGE_DEFAULT,	LAYOUT_DRAW_VERT,	INVALID_PROGRAM, 
			{
				{"LIGHT_SPOT_POINT","1"}
			} 
		},
		{"deferred_light",			SHADER_STAGE_DEFAULT,	LAYOUT_DRAW_VERT,	INVALID_PROGRAM,
			{
				{"LIGHT_DIR","1"}
			}
		},
		{"pp_hdr2ldr",				SHADER_STAGE_DEFAULT,	LAYOUT_DRAW_VERT,	INVALID_PROGRAM, {} },
		{"color",					SHADER_STAGE_DEFAULT,	LAYOUT_DRAW_VERT,	INVALID_PROGRAM, {} },
		{"fxaa3",					SHADER_STAGE_DEFAULT,	LAYOUT_DRAW_VERT,	INVALID_PROGRAM, {} },
		{"emissive_pass",			SHADER_STAGE_DEFAULT,	LAYOUT_DRAW_VERT,	INVALID_PROGRAM, {} },
		{"gauss_filter",			SHADER_STAGE_DEFAULT,	LAYOUT_DRAW_VERT,	INVALID_PROGRAM, {} },
		{"bloom_filter",			SHADER_STAGE_DEFAULT,	LAYOUT_DRAW_VERT,	INVALID_PROGRAM, {} },
		{"ssao_gen",				SHADER_STAGE_DEFAULT,	LAYOUT_DRAW_VERT,	INVALID_PROGRAM, {} },
		{"kernel",					SHADER_STAGE_DEFAULT,	LAYOUT_DRAW_VERT,	INVALID_PROGRAM, {} }
	};

	static std::string GL_map_shader_to_str(GLenum stage)
	{
		switch (stage)
		{
		case GL_VERTEX_SHADER: return "VERTEX";
		case GL_FRAGMENT_SHADER: return "FRAGMENT";
		case GL_GEOMETRY_SHADER: return "GEOMETRY";
		case GL_TESS_CONTROL_SHADER: return "TESS_CONTROL";
		case GL_TESS_EVALUATION_SHADER: return "TESS_EVAL";
		default:
			assert(false);
		}
	}

	static void R_DeleteShaders(GLuint const* list, int numShader)
	{
		for (int i = 0; i < numShader; ++i)
		{
			if (glIsShader(list[i])) { glDeleteShader(list[i]); }
		}
	}

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

	static bool R_CompileShader(GLuint shader, const std::string& name)
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
				Error("Shader '%s' compile error: %s", name.c_str(), logBuf.data());
			}

			return false;
		}

		return true;
	}

	static GLuint R_CreateShader(GLenum stage, const char* name, const std::unordered_map<std::string,std::string>& defines)
	{
		std::string r_name{ name };

		switch (stage)
		{
		case GL_VERTEX_SHADER:
			r_name.append(".vert.glsl");
			break;
		case GL_FRAGMENT_SHADER:
			r_name.append(".frag.glsl");
			break;
		case GL_GEOMETRY_SHADER:
			r_name.append(".geom.glsl");
			break;
		case GL_COMPUTE_SHADER:
			r_name.append(".comp.glsl");
			break;
		}

		GLuint modul = glCreateShader(stage);
		if (!modul) return 0;

		auto source = resourceManager->GetShaderSourceWithVersionAndDefs("shaders/builtin/" + r_name, defines);
		const char* pStr = source.c_str();

		GL_CHECK(glShaderSource(modul, 1, &pStr, nullptr));
		if (R_CompileShader(modul, "ST:" + GL_map_shader_to_str(stage) + "; " + std::string(name)))
		{
			return modul;
		}
		GL_CHECK(glDeleteShader(modul));

		return 0;
	}

	bool ProgramManager::LowLevelInit()
	{
		int idx = 0;
		for (auto& p : builtins)
		{
			Info("[ProgramManager] Creating render program %2d:[%s]", idx++, p.name);
			if (!CreateBuiltinProgram(p))
			{
				Error("[ProgramManager] Cannot initialize render programs");
				for (auto k : builtins)
				{
					if (k.prg != INVALID_PROGRAM)
					{
						GL_CHECK( glDeleteProgram( k.prg ) );
					}
				}
				abort();

				return false;
			}
		}
		return true;
	}
	void ProgramManager::UseProgram(eShaderProg program)
	{
		unsigned int apiObject = builtins[program].prg;
		if (apiObject != currentProgram)
		{
			currentProgram = apiObject;
			GL_CHECK( glUseProgram( (GLuint)apiObject ) );
		}
	}

	void ProgramManager::SetUniform(eShaderProg program, const char* name, int x)
	{
		GLint loc;
		GL_CHECK(loc = glGetUniformLocation(builtins[program].prg, name));
		if (loc > -1)
		{
			glUniform1i(loc, x);
		}
	}

	void ProgramManager::SetUniform(eShaderProg program, const char* name, const glm::vec2& x)
	{
		GLint loc;
		GL_CHECK(loc = glGetUniformLocation(builtins[program].prg, name));
		if (loc > -1)
		{
			glUniform2fv(loc, 1, &x[0]);
		}
	}

	void ProgramManager::SetUniform(eShaderProg program, const char* name, int num, const glm::vec4* v)
	{
		GLint loc;
		GL_CHECK(loc = glGetUniformLocation(builtins[program].prg, name));
		if (loc > -1)
		{
			glUniform4fv(loc, num, &v[0].x);
		}
	}

	void ProgramManager::SetCommonUniform(int idx, const glm::vec4& v)
	{
		assert(idx < 16);
		g_sharedData.params[idx] = v;
	}

	void ProgramManager::UpdateCommonUniform()
	{
		UniformBuffer buf;
		if (renderSystem.vertexCache->GetUniformBuffer(g_commonData_h, buf))
		{
			buf.Update(&g_commonData, 0, sizeof(g_commonData));
		}		
	}

	void ProgramManager::UpdateSharedUniform()
	{
		UniformBuffer buf;
		if (renderSystem.vertexCache->GetUniformBuffer(g_backendData_h, buf))
		{
			buf.Update(&g_sharedData, 0, sizeof(g_sharedData));
		}
	}

	void ProgramManager::BindUniformBlock(eUboBufferBinding binding, const UniformBuffer& buffer)
	{
		GL_CHECK(glBindBufferRange(GL_UNIFORM_BUFFER, binding, buffer.apiObject, buffer.GetOffset(), buffer.GetSize()));
	}

	bool ProgramManager::CreateBuiltinProgram(renderProgram_t& p)
	{

		std::vector<GLuint> shaders;
		if (p.stages & SHADER_STAGE_VERTEX)		shaders.push_back(R_CreateShader(GL_VERTEX_SHADER, p.name, p.defs));
		if (p.stages & SHADER_STAGE_FRAGMENT)	shaders.push_back(R_CreateShader(GL_FRAGMENT_SHADER, p.name, p.defs));
		if (p.stages & SHADER_STAGE_GEOMETRY)	shaders.push_back(R_CreateShader(GL_GEOMETRY_SHADER, p.name, p.defs));
		if (p.stages & SHADER_STAGE_COMPUTE)	shaders.push_back(R_CreateShader(GL_COMPUTE_SHADER, p.name, p.defs));

		bool allOk = true;
		for (auto k : shaders)
		{
			if (k == 0) allOk = false;
		}

		if (!allOk)
		{
			R_DeleteShaders(shaders.data(), shaders.size());
			return false;
		}

		GLuint program = 0xffff;
		allOk = allOk && R_LinkProgram(program, shaders.data(), shaders.size());
		if (allOk && program != 0xffff)
		{
			// All shader stage are compiled
			p.prg = program;
		}

		R_DeleteShaders(shaders.data(), shaders.size());

		return allOk;
	}

	void ProgramManager::Shutdown()
	{
		if (!IsInitialized()) return;

		if (currentProgram)
		{
			glUseProgram(0);
			currentProgram = 0;
		}

		for(auto& k : builtins) 
		{
			GL_CHECK(glDeleteProgram(k.prg));
		}

		initialized = false;
	}

}