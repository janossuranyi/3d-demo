#pragma once

#include <array>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "gpu_types.h"
#include "gpu_utils.h"
#include "gpu_state.h"
#include "gpu_buffer.h"

#define MAX_TEXTURE_UNITS 15
#define MAX_VERTEX_ARRAY_BINDING 32

class Pipeline
{
public:
	Pipeline();
	~Pipeline() = default;

	void setState(uint64_t stateBits, bool forceGlState);
	void setWorldPosition(const glm::vec3& v);
	void setWorldScale(const glm::vec3& v);
	void setWorldEulerRotation(const glm::vec3& v);
	void setWorldQuaternionRotation(const glm::quat& v);
	void setScreenRect(unsigned int x, unsigned int y, unsigned int width, unsigned int height);

	void bindVertexBuffer(GpuBuffer& b, int index = -1, uint32_t offset = 0, uint32_t stride = 0);
	void bindIndexBuffer(GpuBuffer& b);
	void bindUniformBuffer(GpuBuffer& b, int index, uint32_t offset = 0, uint32_t size = 0);
	void drawArrays(eDrawMode mode, int first, uint32_t count);
	void drawElements(eDrawMode mode, uint32_t count, eDataType type, uint32_t offset);
	void drawElements(eDrawMode mode, uint32_t count, eDataType type, uint32_t offset, uint32_t baseVertex);

	void update(float time);

	struct {
		float f_time;
		int i_screen_w;
		int i_screen_h;
		int i_screen_x;
		int i_screen_y;
	} g_misc{};

	struct {
		glm::vec4 v_position;
		glm::vec4 v_direction;
		glm::vec4 v_color;
		float f_intensity;
	} g_sun{};

	struct {
		glm::vec4 v_position;
		glm::vec4 v_target;
		glm::vec4 v_direction;
		glm::vec4 v_up;
		glm::vec4 v_near_far_fov;
	} g_cam{};

	struct {
		glm::mat4 m_W;
		glm::mat4 m_V;
		glm::mat4 m_P;
		glm::mat4 m_Normal;

		glm::mat4 m_WV;
		glm::mat4 m_VP;
		glm::mat4 m_WVP;

		glm::mat4 m_iP;
		glm::mat4 m_iVP;
	} g_mtx{};

	struct {
		glm::vec4 v_highFreq[32];
	} cb_highFreq;

	struct {
		glm::vec4 v_lowFreq[32];
	} cb_lowFreq;

	struct indexedBufferBinding_t {
		GLuint buffer;
		uint32_t offset;
		uint32_t stride;
	};
private:

	GLfloat _polyOfsScale, _polyOfsBias;
	GLuint64 m_glStateBits;

	struct tmu_t {
		GLuint texId;
		GLuint target;
	} m_tmus[MAX_TEXTURE_UNITS];

	GLuint m_activeArrayBuffer;
	GLuint m_activeElementBuffer;
	GLuint m_activeVertexArray;
	GLuint m_activeFrameBuffer;

	std::array<indexedBufferBinding_t, MAX_VERTEX_ARRAY_BINDING> m_activeVertexArrayBinding;
	std::array<indexedBufferBinding_t, MAX_VERTEX_ARRAY_BINDING> m_activeUniformArrayBinding;

	glm::vec3 m_worldPosition;
	glm::vec3 m_worldScale;
	glm::quat m_worldRotation;
	glm::vec3 m_worldEulerAngles;
};