#include <memory>
#include <cstdio>
#include <string>
#include "demo.h"
#include "engine/resource/filesystem.h"
#include "logger.h"
#include "gpu_types.h"
#include "gpu_utils.h"
#include "gpu_program.h"

#undef _std
#define _std std::

using namespace rc;

GpuProgram::GpuProgram()
{
	_program = 0;
}

GpuProgram& GpuProgram::operator=(GpuProgram&& moved) noexcept
{
	_program = moved._program;
	moved._program = 0;

	return *this;
}
GpuProgram::GpuProgram(GpuProgram&& moved) noexcept
{
	_program = moved._program;
	moved._program = 0;
}
GpuProgram::~GpuProgram()
{
	destroy();
}
bool GpuProgram::bindUniformBlock(const std::string& name, int bindingIndex)
{
	unsigned int var_index;
	GL_CHECK(var_index = glGetUniformBlockIndex(_program, name.c_str()));
	if (var_index == GL_INVALID_INDEX) return false;

	GL_CHECK(glUniformBlockBinding(_program, var_index, bindingIndex));

	return true;
}
bool GpuProgram::loadShader(const std::string& vertexShader, const std::string& fragmentShader)
{
	std::string vs, fs;
	if (!FileSystem::read_text_file(vertexShader, vs))		return false;
	if (!FileSystem::read_text_file(fragmentShader, fs))	return false;

	std::vector<const char*> vs_c = { vs.c_str() };
	std::vector<const char*> fs_c = { fs.c_str() };
	return createProgramFromShaderSource(vs_c, fs_c);

}

bool GpuProgram::loadComputeShader(const std::string& shader)
{
	std::string cs;
	if (!FileSystem::read_text_file(shader, cs))		return false;

	std::vector<const char*> cs_c = { cs.c_str() };
	return createComputeProgramFromShaderSource(cs_c);
}

GLuint GpuProgram::createShaderInternal(ShaderType stage, const std::vector<const char*>& sources)
{
	GLuint shader;
	GL_CHECK(shader = glCreateShader(GL_castShaderStage(stage)));

	if (!shader) return 0;

	GL_CHECK(glShaderSource(shader, sources.size(), sources.data(), nullptr));

	return shader;
}


bool GpuProgram::createProgramFromShaderSource(const std::vector<const char*>& vert_sources, const std::vector<const char*>& frag_sources)
{
	if (_program != 0xFFFF)
	{
		GL_CHECK(glDeleteProgram(_program));
	}

	GL_CHECK(_program = glCreateProgram());

	GLuint vertShader = createShaderInternal(ShaderType::VERTEX, vert_sources);

	if (!compileSingleStage(vertShader, ShaderType::VERTEX))
	{
		destroy();
		return false;
	}

	GLuint fragShader = createShaderInternal(ShaderType::FRAGMENT, frag_sources);

	if (!compileSingleStage(fragShader, ShaderType::FRAGMENT))
	{
		GL_CHECK(glDeleteShader(vertShader));
		destroy();
		return false;
	}

	GL_CHECK(glAttachShader(_program, vertShader));
	GL_CHECK(glAttachShader(_program, fragShader));
	GL_CHECK(glLinkProgram(_program));

	GL_CHECK(glDeleteShader(vertShader));
	GL_CHECK(glDeleteShader(fragShader));

	GLint result = GL_FALSE;

	GL_CHECK(glGetProgramiv(_program, GL_LINK_STATUS, &result));

	if (result == GL_FALSE)
	{
		GLint infologLen;
		GL_CHECK(glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &infologLen));
		if (infologLen > 0) {
			std::vector<char> logBuf(infologLen);
			GL_CHECK(glGetProgramInfoLog(_program, infologLen, nullptr, logBuf.data()));
			Error("Linking of shader program failed: %s", logBuf.data());

			destroy();
			return false;
		}
	}

	return true;
}
bool GpuProgram::createComputeProgramFromShaderSource(const std::vector<const char*>& sources)
{
	if (_program != 0)
	{
		GL_CHECK(glDeleteProgram(_program));
	}

	GL_CHECK(_program = glCreateProgram());

	GLuint shader = createShaderInternal(ShaderType::COMPUTE, sources);

	if (!compileSingleStage(shader, ShaderType::COMPUTE))
	{
		destroy();
		return false;
	}

	GL_CHECK(glAttachShader(_program, shader));
	GL_CHECK(glLinkProgram(_program));
	GL_CHECK(glDeleteShader(shader));
	GLint result = GL_FALSE;

	GL_CHECK(glGetProgramiv(_program, GL_LINK_STATUS, &result));

	if (result == GL_FALSE)
	{
		GLint infologLen;
		GL_CHECK(glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &infologLen));
		if (infologLen > 0) {
			std::vector<char> logBuf(infologLen);
			GL_CHECK(glGetProgramInfoLog(_program, infologLen, nullptr, logBuf.data()));
			Error("Linking of shader program failed: %s", logBuf.data());

			destroy();
			return false;
		}
	}

	_isComputeShader = true;
	return true;

}

int GpuProgram::getLocation(const std::string& name) const
{
	int r;
	GL_CHECK(r = glGetUniformLocation(_program, name.c_str()));

	return r;
}

bool GpuProgram::mapLocationToIndex(const std::string& name, const int index)
{

	GLint loc = getLocation(name);
	if (loc < 0)
	{
		return false;
	}

	if (_locationBindings.size() <= index)
	{
		_locationBindings.resize(index + 1);
	}

	_locationBindings[index] = loc;

}

void GpuProgram::set(int index, float f) const
{
	assert(index < _locationBindings.size());
	GL_CHECK(glUniform1f(_locationBindings[index], f));
}
void GpuProgram::set(int index, int n, const float* f) const
{
	assert(index < _locationBindings.size());
	GL_CHECK(glUniform1fv(_locationBindings[index], n, f));
}
void GpuProgram::set(int index, int i) const
{
	assert(index < _locationBindings.size());
	GL_CHECK(glUniform1i(_locationBindings[index], i));
}
void GpuProgram::set(int index, bool transpose, const glm::mat4& m) const
{
	assert(index < _locationBindings.size());
	GL_CHECK(glUniformMatrix4fv(_locationBindings[index], 1, GLboolean(transpose), &m[0][0]));
}
void GpuProgram::set(int index, bool transpose, const glm::mat3& m) const
{
	assert(index < _locationBindings.size());
	GL_CHECK(glUniformMatrix3fv(_locationBindings[index], 1, GLboolean(transpose), &m[0][0]));
}
void GpuProgram::set(int index, bool transpose, int n, const glm::mat4* m) const
{
	assert(index < _locationBindings.size());
	GL_CHECK(glUniformMatrix4fv(_locationBindings[index], n, GLboolean(transpose), reinterpret_cast<const GLfloat*>(m)));
}
void GpuProgram::set(int index, bool transpose, int n, const glm::mat3* m) const
{
	assert(index < _locationBindings.size());
	GL_CHECK(glUniformMatrix3fv(_locationBindings[index], n, GLboolean(transpose), reinterpret_cast<const GLfloat*>(m)));
}
void GpuProgram::set(int index, const glm::vec2& v) const
{
	assert(index < _locationBindings.size());
	GL_CHECK(glUniform2fv(_locationBindings[index], 1, &v[0]));
}
void GpuProgram::set(int index, const glm::vec3& v) const
{
	assert(index < _locationBindings.size());
	GL_CHECK(glUniform3fv(_locationBindings[index], 1, &v[0]));
}
void GpuProgram::set(int index, const glm::vec4& v) const
{
	assert(index < _locationBindings.size());
	GL_CHECK(glUniform4fv(_locationBindings[index], 1, &v[0]));
}
void GpuProgram::set(int index, int n, const glm::vec2* v) const
{
	assert(index < _locationBindings.size());
	GL_CHECK(glUniform2fv(_locationBindings[index], n, reinterpret_cast<const GLfloat*>(v)));
}
void GpuProgram::set(int index, int n, const glm::vec3* v) const
{
	assert(index < _locationBindings.size());
	GL_CHECK(glUniform3fv(_locationBindings[index], n, reinterpret_cast<const GLfloat*>(v)));
}
void GpuProgram::set(int index, int n, const glm::vec4* v) const
{
	assert(index < _locationBindings.size());
	GL_CHECK(glUniform4fv(_locationBindings[index], n, reinterpret_cast<const GLfloat*>(v)));
}
void GpuProgram::set(const std::string& name, int i) const
{
	GLint loc = getLocation(name);
	if (loc > -1)
	{
		glUniform1i(loc, i);
	}
}
void GpuProgram::set(const std::string& name, float i) const
{
	GLint loc = getLocation(name);
	if (loc > -1)
	{
		glUniform1f(loc, i);
	}
}
void GpuProgram::set(const std::string& name, const glm::ivec2& i) const
{
	GLint loc = getLocation(name);
	if (loc > -1)
	{
		glUniform2iv(loc, 1, &i[0]);
	}
}
void GpuProgram::set(const std::string& name, const glm::vec2& i) const
{
	GLint loc = getLocation(name);
	if (loc > -1)
	{
		glUniform2fv(loc, 1, &i[0]);
	}
}
void GpuProgram::use() const
{
	assert(_program != 0xFFFF);
	GL_CHECK(glUseProgram(_program));
}
void GpuProgram::dispatchCompute(unsigned int x, unsigned int y, unsigned int z) const
{
	assert(_program != 0);
	assert(_isComputeShader == true);
	GL_CHECK(glDispatchCompute(x, y, z));
}
void GpuProgram::destroy()
{
	if (_program != 0)
	{
		GL_CHECK(glDeleteProgram(_program));
		_program = 0;
	}
}
bool GpuProgram::compileSingleStage(GLuint shaderId, ShaderType type)
{
	GLint result = GL_FALSE;

	GL_CHECK(glCompileShader(shaderId));
	GL_CHECK(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result));

	if (result == GL_FALSE)
	{
		GLint infologLen;
		GL_CHECK(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infologLen));
		if (infologLen > 0) {
			std::vector<char> logBuf(infologLen);
			GL_CHECK(glGetShaderInfoLog(shaderId, infologLen, nullptr, logBuf.data()));
			const char* sType = GetShaderStageTitle(type);
			Error("%s shader compilation failed: %s", sType, logBuf.data());
		}

		GL_CHECK(glDeleteShader(shaderId));

		return false;
	}

	return true;
}


