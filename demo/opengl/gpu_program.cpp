#include <memory>
#include <cstdio>
#include <string>
#include "demo.h"
#include "filesystem.h"
#include "logger.h"
#include "gpu_types.h"
#include "gpu_utils.h"
#include "gpu_program.h"

#undef _std
#define _std std::



GpuProgram::GpuProgram()
{
	mProgId = 0xffff;
}

GpuProgram& GpuProgram::operator=(GpuProgram&& moved)
{
	mProgId = moved.mProgId;
	moved.mProgId = 0xFFFF;

	return *this;
}
GpuProgram::GpuProgram(GpuProgram&& moved)
{
	mProgId = moved.mProgId;
	moved.mProgId = 0xFFFF;
}
GpuProgram::~GpuProgram()
{
	destroy();
}
bool GpuProgram::bindUniformBlock(const std::string& name, int bindingIndex)
{
	unsigned int var_index;
	GL_CHECK(var_index = glGetUniformBlockIndex(mProgId, name.c_str()));
	if (var_index == GL_INVALID_INDEX) return false;

	GL_CHECK(glUniformBlockBinding(mProgId, var_index, bindingIndex));

	return true;
}
bool GpuProgram::loadShader(const std::string& vertexShader, const std::string& fragmentShader)
{
	std::string vs, fs;
	if (!g_fileSystem.read_text_file(vertexShader, vs))		return false;
	if (!g_fileSystem.read_text_file(fragmentShader, fs))	return false;

	std::vector<const char*> vs_c = { vs.c_str() };
	std::vector<const char*> fs_c = { fs.c_str() };
	return createProgramFromShaderSource(vs_c, fs_c);

}

bool GpuProgram::loadComputeShader(const std::string& shader)
{
	std::string cs;
	if (!g_fileSystem.read_text_file(shader, cs))		return false;

	std::vector<const char*> cs_c = { cs.c_str() };
	return createComputeProgramFromShaderSource(cs_c);
}

GLuint GpuProgram::createShaderInternal(eShaderStage stage, const std::vector<const char*>& sources)
{
	GLuint shader;
	GL_CHECK(shader = glCreateShader(GL_castShaderStage(stage)));

	if (!shader) return 0;

	GL_CHECK(glShaderSource(shader, sources.size(), sources.data(), nullptr));

	return shader;
}


bool GpuProgram::createProgramFromShaderSource(const std::vector<const char*>& vert_sources, const std::vector<const char*>& frag_sources)
{
	if (mProgId != 0xFFFF)
	{
		GL_CHECK(glDeleteProgram(mProgId));
	}

	GL_CHECK(mProgId = glCreateProgram());

	GLuint vertShader = createShaderInternal(eShaderStage::VERTEX, vert_sources);

	if (!compileSingleStage(vertShader, eShaderStage::VERTEX))
	{
		destroy();
		return false;
	}

	GLuint fragShader = createShaderInternal(eShaderStage::FRAGMENT, frag_sources);

	if (!compileSingleStage(fragShader, eShaderStage::FRAGMENT))
	{
		GL_CHECK(glDeleteShader(vertShader));
		destroy();
		return false;
	}

	GL_CHECK(glAttachShader(mProgId, vertShader));
	GL_CHECK(glAttachShader(mProgId, fragShader));
	GL_CHECK(glLinkProgram(mProgId));

	GL_CHECK(glDeleteShader(vertShader));
	GL_CHECK(glDeleteShader(fragShader));

	GLint result = GL_FALSE;

	GL_CHECK(glGetProgramiv(mProgId, GL_LINK_STATUS, &result));

	if (result == GL_FALSE)
	{
		GLint infologLen;
		GL_CHECK(glGetProgramiv(mProgId, GL_INFO_LOG_LENGTH, &infologLen));
		if (infologLen > 0) {
			std::vector<char> logBuf(infologLen);
			GL_CHECK(glGetProgramInfoLog(mProgId, infologLen, nullptr, logBuf.data()));
			Error("Linking of shader program failed: %s", logBuf.data());

			destroy();
			return false;
		}
	}

	return true;
}
bool GpuProgram::createComputeProgramFromShaderSource(const std::vector<const char*>& sources)
{
	if (mProgId != 0xFFFF)
	{
		GL_CHECK(glDeleteProgram(mProgId));
	}

	GL_CHECK(mProgId = glCreateProgram());

	GLuint shader = createShaderInternal(eShaderStage::COMPUTE, sources);

	if (!compileSingleStage(shader, eShaderStage::COMPUTE))
	{
		destroy();
		return false;
	}

	GL_CHECK(glAttachShader(mProgId, shader));
	GL_CHECK(glLinkProgram(mProgId));
	GL_CHECK(glDeleteShader(shader));
	GLint result = GL_FALSE;

	GL_CHECK(glGetProgramiv(mProgId, GL_LINK_STATUS, &result));

	if (result == GL_FALSE)
	{
		GLint infologLen;
		GL_CHECK(glGetProgramiv(mProgId, GL_INFO_LOG_LENGTH, &infologLen));
		if (infologLen > 0) {
			std::vector<char> logBuf(infologLen);
			GL_CHECK(glGetProgramInfoLog(mProgId, infologLen, nullptr, logBuf.data()));
			Error("Linking of shader program failed: %s", logBuf.data());

			destroy();
			return false;
		}
	}

	return true;

}

int GpuProgram::getLocation(const std::string& name) const
{
	int r;
	GL_CHECK(r = glGetUniformLocation(mProgId, name.c_str()));

	return r;
}

bool GpuProgram::mapLocationToIndex(const std::string& name, const int index)
{

	GLint loc = getLocation(name);
	if (loc < 0)
	{
		return false;
	}

	if (mMapVar.size() <= index)
	{
		mMapVar.resize(index + 1);
	}

	mMapVar[index] = loc;

}

void GpuProgram::set(int index, float f) const
{
	assert(index < mMapVar.size());
	GL_CHECK(glUniform1f(mMapVar[index], f));
}
void GpuProgram::set(int index, int n, const float* f) const
{
	assert(index < mMapVar.size());
	GL_CHECK(glUniform1fv(mMapVar[index], n, f));
}
void GpuProgram::set(int index, int i) const
{
	assert(index < mMapVar.size());
	GL_CHECK(glUniform1i(mMapVar[index], i));
}
void GpuProgram::set(int index, bool transpose, const glm::mat4& m) const
{
	assert(index < mMapVar.size());
	GL_CHECK(glUniformMatrix4fv(mMapVar[index], 1, GLboolean(transpose), &m[0][0]));
}
void GpuProgram::set(int index, bool transpose, const glm::mat3& m) const
{
	assert(index < mMapVar.size());
	GL_CHECK(glUniformMatrix3fv(mMapVar[index], 1, GLboolean(transpose), &m[0][0]));
}
void GpuProgram::set(int index, bool transpose, int n, const glm::mat4* m) const
{
	assert(index < mMapVar.size());
	GL_CHECK(glUniformMatrix4fv(mMapVar[index], n, GLboolean(transpose), reinterpret_cast<const GLfloat*>(m)));
}
void GpuProgram::set(int index, bool transpose, int n, const glm::mat3* m) const
{
	assert(index < mMapVar.size());
	GL_CHECK(glUniformMatrix3fv(mMapVar[index], n, GLboolean(transpose), reinterpret_cast<const GLfloat*>(m)));
}
void GpuProgram::set(int index, const glm::vec2& v) const
{
	assert(index < mMapVar.size());
	GL_CHECK(glUniform2fv(mMapVar[index], 1, &v[0]));
}
void GpuProgram::set(int index, const glm::vec3& v) const
{
	assert(index < mMapVar.size());
	GL_CHECK(glUniform3fv(mMapVar[index], 1, &v[0]));
}
void GpuProgram::set(int index, const glm::vec4& v) const
{
	assert(index < mMapVar.size());
	GL_CHECK(glUniform4fv(mMapVar[index], 1, &v[0]));
}
void GpuProgram::set(int index, int n, const glm::vec2* v) const
{
	assert(index < mMapVar.size());
	GL_CHECK(glUniform2fv(mMapVar[index], n, reinterpret_cast<const GLfloat*>(v)));
}
void GpuProgram::set(int index, int n, const glm::vec3* v) const
{
	assert(index < mMapVar.size());
	GL_CHECK(glUniform3fv(mMapVar[index], n, reinterpret_cast<const GLfloat*>(v)));
}
void GpuProgram::set(int index, int n, const glm::vec4* v) const
{
	assert(index < mMapVar.size());
	GL_CHECK(glUniform4fv(mMapVar[index], n, reinterpret_cast<const GLfloat*>(v)));
}
void GpuProgram::use() const
{
	assert(mProgId != 0xFFFF);
	GL_CHECK(glUseProgram(mProgId));
}
void GpuProgram::destroy()
{
	if (mProgId != 0xFFFF)
	{
		GL_CHECK(glDeleteProgram(mProgId));
		mProgId = 0xFFFF;
	}
}
bool GpuProgram::compileSingleStage(GLuint shaderId, eShaderStage type)
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


