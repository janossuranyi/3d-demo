#pragma once
#include <vector>
#include <string>
#include <memory>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "gpu_types.h"

class GpuProgram
{
public:

	GpuProgram();
	GpuProgram(GpuProgram& copied) = delete;
	GpuProgram& operator=(GpuProgram&) = delete;
	GpuProgram& operator=(GpuProgram&&);
	GpuProgram(GpuProgram&& moved);
	~GpuProgram();

	bool bindUniformBlock(const std::string& name, int index);
	bool loadShader(const std::string& vertexShader, const std::string& fragmentShader);
	bool loadComputeShader(const std::string& shader);

	bool createProgramFromShaderSource(const std::vector<const char*>& vert_sources, const std::vector<const char*>& frag_sources);
	bool createComputeProgramFromShaderSource(const std::vector<const char*>&  sources);

	int getLocation(const std::string& name) const;
	bool mapLocationToIndex(const std::string& name, const int index);

	void set(int index, float f) const;
	void set(int index, int n, const float* f) const;
	void set(int index, int i) const;
	void set(int index, bool transpose, const glm::mat4& m) const;
	void set(int index, bool transpose, const glm::mat3& m) const;
	void set(int index, bool transpose, int n, const glm::mat4 *m) const;
	void set(int index, bool transpose, int n, const glm::mat3* m) const;
	void set(int index, const glm::vec2& v) const;
	void set(int index, const glm::vec3& v) const;
	void set(int index, const glm::vec4& v) const;
	void set(int index, int n, const glm::vec2* v) const;
	void set(int index, int n, const glm::vec3* v) const;
	void set(int index, int n, const glm::vec4* v) const;

	void use() const;
	void destroy();

private:
	GLuint programId() const { return mProgId; }
	GLuint createShaderInternal(eShaderStage stage, const std::vector<const char*>& sources);
	bool compileSingleStage(GLuint shaderId, eShaderStage type);
	GLuint mProgId;
	std::vector<GLint> mMapVar;
};
