#include "gfx/opengl/gl_context.h"
#include "gfx/opengl/gl_helper.h"

#include "logger.h"

namespace gfx {
	void OpenGLRenderContext::operator()(const cmd::CreateShader& cmd)
	{
		if (shader_map_.count(cmd.handle) > 0)
			return;

		GLuint shader = 0xffff;
		GL_CHECK(shader = glCreateShader(MapShaderStage(cmd.stage)));

		assert(shader != 0xffff);

		const GLchar* tmp = cmd.source.c_str();
		GL_CHECK(glShaderSource(shader, 1, &tmp, nullptr));

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
				const char* sType = MapShaderStageTitle(cmd.stage);
				Error("%s shader compilation failed: %s", sType, logBuf.data());
			}

			GL_CHECK(glDeleteShader(shader));
			return;
		}

		shader_map_.emplace(cmd.handle, ShaderData{ shader, cmd.source, cmd.stage, true });
	}

	void OpenGLRenderContext::operator()(const cmd::DeleteShader& cmd)
	{
		auto& result = shader_map_.find(cmd.handle);
		GL_CHECK(glDeleteShader(result->second.shader));
		shader_map_.erase(cmd.handle);

#ifdef _DEBUG
		Info("Shader %d deleted", cmd.handle);
#endif

	}

	void OpenGLRenderContext::operator()(const cmd::CreateProgram& cmd)
	{
		GLuint program;
		GL_CHECK(program = glCreateProgram());
		if (program)
		{
			const ProgramData data = { program, false, {} };
			program_map_.emplace(cmd.handle, data);
		}
	}

	void OpenGLRenderContext::operator()(const cmd::LinkProgram& cmd)
	{
		auto& p_data = program_map_.at(cmd.handle);
		for (auto& s_handle : cmd.shaders)
		{
			auto& s_data = shader_map_.find(s_handle);
			if (s_data == std::end(shader_map_))
			{
				return;
			}
			GL_CHECK(glAttachShader(p_data.program, s_data->second.shader));
		}

		p_data.linked = false;
		GL_CHECK(glLinkProgram(p_data.program));
		GLint result = GL_FALSE;

		GL_CHECK(glGetProgramiv(p_data.program, GL_LINK_STATUS, &result));

		if (result == GL_FALSE)
		{
			GLint infologLen;
			GL_CHECK(glGetProgramiv(p_data.program, GL_INFO_LOG_LENGTH, &infologLen));
			if (infologLen > 0) {
				std::vector<char> logBuf(infologLen);
				GL_CHECK(glGetProgramInfoLog(p_data.program, infologLen, nullptr, logBuf.data()));
				Error("Linking of shader program failed: %s", logBuf.data());
			}
		}
		else
		{
			p_data.linked = true;
		}

		for (auto& s_handle : cmd.shaders)
		{
			auto& s_data = shader_map_.at(s_handle);
			GL_CHECK(glDetachShader(p_data.program, s_data.shader));
		}

	}

	void OpenGLRenderContext::operator()(const cmd::DeleteProgram& cmd)
	{
		auto& p_data = program_map_.at(cmd.handle);
		GL_CHECK(glDeleteProgram(p_data.program));
		program_map_.erase(cmd.handle);

#ifdef _DEBUG
		Info("Program %d deleted", cmd.handle);
#endif

	}

}