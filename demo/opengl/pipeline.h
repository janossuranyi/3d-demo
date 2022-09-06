#pragma once

#include <array>
#include <memory>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "gpu_types.h"
#include "gpu_utils.h"
#include "gpu_state.h"
#include "gpu_buffer.h"
#include "gpu_texture.h"
#include "gpu_program.h"
#include "gpu_vertex_layout.h"

#include "camera.h"
#include "light.h"
#include "material.h"
#include "world.h"

#define MAX_TEXTURE_UNITS 15
#define MAX_BUFFER_BINDING 32

const int CB_MATRIX = 1;
const int CB_CAMERA = 2;
const int CB_SUN = 3;
const int CB_MISC = 4;

const int TEX_ALBEDO = 0;
const int TEX_NORMAL = 1;
const int TEX_PBR = 2;
const int TEX_EMISSIVE = 3;
const int TEX_AO = 4;

class Pipeline
{
public:
	Pipeline();
	~Pipeline();

	void setState(uint64_t stateBits, bool forceGlState = false);
	void setWorldPosition(const vec3& v);
	void setWorldScale(const vec3& v);
	void setWorldEulerRotation(const vec3& v);
	void setWorldQuaternionRotation(const quat& v);
	void setWorldMatrix(const mat4& worldMtx);
	void setScreenRect(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
	void setLayout(const VertexLayout& layout);
	void useProgram(GpuProgram& prog);
	void bindVertexBuffer(GpuBuffer& b, int index = -1, uint32_t offset = 0, uint32_t stride = 0);
	void bindIndexBuffer(const GpuBuffer& b);
	void bindUniformBuffer(GpuBuffer& b, int index, uint32_t offset = 0, uint32_t size = 0);
	void drawArrays(eDrawMode mode, int first, uint32_t count);
	void drawElements(eDrawMode mode, uint32_t count, eDataType type, uint32_t offset);
	void drawElements(eDrawMode mode, uint32_t count, eDataType type, uint32_t offset, uint32_t baseVertex);
	void bindTexture(GpuTexture2D& tex, int unit);

	void setMaterial(Material& material, World& world);
	void setView(const vec3& pos, const vec3& target);
	void setClearColor(float r, float b, float g, float a);
	void setClearDepth(float d);
	void setPerspectiveCamera(float yfov, float znear, float zfar, float aspect);

	void depthTestEnable(bool b) const;
	void update();
	void clear(bool color, bool depth, bool stencil);
	void bindConstantBuffers();


	struct alignas(16) g_material_t {
		glm::vec4 baseColor;
		glm::vec4 specularColor;
		glm::vec4 emissiveColor;
		float param1;   // metalness
		float param2;   // roughness/shininess
		glm::uint flags;
	} g_material;

	struct alignas(16) g_misc_t {
		float time;
		int screen_w;
		int screen_h;
		int screen_x;
		int screen_y;
	} g_misc;

	struct alignas(16) g_sun_t {
		glm::vec4 position;
		glm::vec4 direction;
		glm::vec4 color;
	} g_sun;

	struct alignas(16) g_cam_t {
		glm::vec4 position;
		glm::vec4 target;
		glm::vec4 direction;
		glm::vec4 up;
		float znear;
		float zfar;
		float yfov;
		float ascept;
	} g_cam;

	struct alignas(16) g_mtx_t {
		glm::mat4 m_W;
		glm::mat4 m_V;
		glm::mat4 m_P;
		glm::mat4 m_Normal;

		glm::mat4 m_WV;
		glm::mat4 m_VP;
		glm::mat4 m_WVP;

		glm::mat4 m_iP;
		glm::mat4 m_iVP;
	} g_mtx;

	struct alignas(16) {
		glm::vec4 v_highFreq[16];
	} cb_highFreq;

	struct alignas(16) {
		glm::vec4 v_lowFreq[16];
	} cb_lowFreq;

	struct indexedBufferBinding_t {
		GLuint buffer;
		uint32_t offset;
		union {
			uint32_t stride;
			uint32_t size;
		};
	};
private:

	GLfloat _polyOfsScale, _polyOfsBias;
	GLuint64 m_glStateBits{};

	struct tmu_t {
		GLuint texId;
		GLuint target;
	} m_tmus[MAX_TEXTURE_UNITS];

	GLuint m_activeArrayBuffer{};
	GLuint m_activeElementBuffer{};
	GLuint m_activeVertexArray{};
	GLuint m_activeFrameBuffer{};
	GLuint m_activeProgram{};

	std::array<indexedBufferBinding_t, MAX_BUFFER_BINDING> m_activeVertexArrayBinding{};
	std::array<indexedBufferBinding_t, MAX_BUFFER_BINDING> m_activeUniformBinding{};

	std::unique_ptr<GpuBuffer> m_mtxBuffer;
	std::unique_ptr<GpuBuffer> m_camBuffer;
	std::unique_ptr<GpuBuffer> m_sunBuffer;
	std::unique_ptr<GpuBuffer> m_miscBuffer;

	vec3 m_worldPosition{};
	vec3 m_worldScale{};
	quat m_worldRotation{};
	vec3 m_worldEulerAngles{};

	bool m_bChangeWVP{};
	bool m_bChangeView{};
	bool m_bChangeSun{};
	bool m_bChangeCam{};

	GLint m_activeLayout;

	void setConstantBuffer(int name, GpuBuffer* buffer);
	void init();

};