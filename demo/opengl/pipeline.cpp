#include <SDL.h>
#include <cstring>
#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "pipeline.h"

Pipeline::Pipeline()
{
	g_misc.time = SDL_static_cast(float, SDL_GetTicks64());
	g_misc.screen_w = 1920;
	g_misc.screen_h = 1080;
	g_misc.screen_x = 0;
	g_misc.screen_y = 0;

	g_sun.position = { 5, 5, -5, 1 };
	g_sun.direction = glm::normalize(float4(0,0,0,1) - g_sun.position);
	g_sun.direction.w = 0.0f;
	g_sun.color = float4(1, 1, 1, 1);

	g_cam.position = glm::vec4(0, 0, -5, 1);
	g_cam.direction = glm::normalize(float4(0,0,0,1) - g_cam.position);
	g_cam.direction.w = 0.0f;
	g_cam.up = glm::vec4(0, 1, 0, 0);
	g_cam.znear = 0.01f;
	g_cam.zfar = 100.0f;
	g_cam.yfov = 45.0f;
	g_cam.ascept = float(g_misc.screen_w) / g_misc.screen_h;

	m_activeArrayBuffer = 0;
	m_activeElementBuffer = 0;
	m_activeVertexArray = 0;
	m_activeFrameBuffer = 0;
	m_activeProgram = 0;
	_polyOfsBias = 0.0f;
	_polyOfsScale = 0.0f;

	for (int i = 0; i < MAX_TEXTURE_UNITS; ++i)
	{
		m_tmus[i].target = 0;
		m_tmus[i].texId = 0;
	}

	m_camBuffer = new GpuBuffer(eGpuBufferTarget::UNIFORM);
	m_sunBuffer = new GpuBuffer(eGpuBufferTarget::UNIFORM);
	m_miscBuffer = new GpuBuffer(eGpuBufferTarget::UNIFORM);
	m_mtxBuffer = new GpuBuffer(eGpuBufferTarget::UNIFORM);

#define CREATE_CB_BUFFER(p,t) p->create(sizeof(t), eGpuBufferUsage::DYNAMIC, BA_WRITE_PERSISTENT_COHERENT, &t)

	CREATE_CB_BUFFER(m_camBuffer, g_cam);
	CREATE_CB_BUFFER(m_mtxBuffer, g_mtx);
	CREATE_CB_BUFFER(m_miscBuffer, g_misc);
	CREATE_CB_BUFFER(m_sunBuffer, g_sun);

	init();

}

Pipeline::~Pipeline()
{
	delete m_camBuffer;
	delete m_mtxBuffer;
	delete m_miscBuffer;
	delete m_sunBuffer;
}


void Pipeline::setConstantBuffer(int name, GpuBuffer* buffer)
{
	switch (name)
	{
	case CB_MATRIX:
		m_mtxBuffer = buffer;
		break;
	case CB_CAMERA:
		m_camBuffer = buffer;
		break;
	case CB_SUN:
		m_sunBuffer = buffer;
		break;
	case CB_MISC:
		m_miscBuffer = buffer;
		break;
	}
}

void Pipeline::setState(uint64_t stateBits, bool forceGlState)
{
	uint64_t diff = stateBits ^ m_glStateBits;

	if (forceGlState)
	{
		// make sure everything is set all the time, so we
		// can see if our delta checking is screwing up
		diff = 0xFFFFFFFFFFFFFFFF;
	}
	else if (diff == 0)
	{
		return;
	}

	//
	// culling
	//
	if (diff & (GLS_CULL_MASK))//| GLS_MIRROR_VIEW ) )
	{
		switch (stateBits & GLS_CULL_MASK)
		{
		case GLS_CULL_TWOSIDED:
			glDisable(GL_CULL_FACE);
			break;

		case GLS_CULL_BACKSIDED:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			break;

		case GLS_CULL_FRONTSIDED:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			break;
		}
	}

	//
	// check depthFunc bits
	//
	if (diff & GLS_DEPTHFUNC_BITS)
	{
		switch (stateBits & GLS_DEPTHFUNC_BITS)
		{
		case GLS_DEPTHFUNC_EQUAL:
			glDepthFunc(GL_EQUAL);
			break;
		case GLS_DEPTHFUNC_ALWAYS:
			glDepthFunc(GL_ALWAYS);
			break;
		case GLS_DEPTHFUNC_LESS:
			glDepthFunc(GL_LEQUAL);
			break;
		case GLS_DEPTHFUNC_GREATER:
			glDepthFunc(GL_GEQUAL);
			break;
		}
	}

	//
	// check blend bits
	//
	if (diff & (GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS))
	{
		GLenum srcFactor = GL_ONE;
		GLenum dstFactor = GL_ZERO;

		switch (stateBits & GLS_SRCBLEND_BITS)
		{
		case GLS_SRCBLEND_ZERO:
			srcFactor = GL_ZERO;
			break;
		case GLS_SRCBLEND_ONE:
			srcFactor = GL_ONE;
			break;
		case GLS_SRCBLEND_DST_COLOR:
			srcFactor = GL_DST_COLOR;
			break;
		case GLS_SRCBLEND_ONE_MINUS_DST_COLOR:
			srcFactor = GL_ONE_MINUS_DST_COLOR;
			break;
		case GLS_SRCBLEND_SRC_ALPHA:
			srcFactor = GL_SRC_ALPHA;
			break;
		case GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA:
			srcFactor = GL_ONE_MINUS_SRC_ALPHA;
			break;
		case GLS_SRCBLEND_DST_ALPHA:
			srcFactor = GL_DST_ALPHA;
			break;
		case GLS_SRCBLEND_ONE_MINUS_DST_ALPHA:
			srcFactor = GL_ONE_MINUS_DST_ALPHA;
			break;
		default:
			assert(!"GL_State: invalid src blend state bits\n");
			break;
		}

		switch (stateBits & GLS_DSTBLEND_BITS)
		{
		case GLS_DSTBLEND_ZERO:
			dstFactor = GL_ZERO;
			break;
		case GLS_DSTBLEND_ONE:
			dstFactor = GL_ONE;
			break;
		case GLS_DSTBLEND_SRC_COLOR:
			dstFactor = GL_SRC_COLOR;
			break;
		case GLS_DSTBLEND_ONE_MINUS_SRC_COLOR:
			dstFactor = GL_ONE_MINUS_SRC_COLOR;
			break;
		case GLS_DSTBLEND_SRC_ALPHA:
			dstFactor = GL_SRC_ALPHA;
			break;
		case GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA:
			dstFactor = GL_ONE_MINUS_SRC_ALPHA;
			break;
		case GLS_DSTBLEND_DST_ALPHA:
			dstFactor = GL_DST_ALPHA;
			break;
		case GLS_DSTBLEND_ONE_MINUS_DST_ALPHA:
			dstFactor = GL_ONE_MINUS_DST_ALPHA;
			break;
		default:
			assert(!"GL_State: invalid dst blend state bits\n");
			break;
		}

		// Only actually update GL's blend func if blending is enabled.
		if (srcFactor == GL_ONE && dstFactor == GL_ZERO)
		{
			glDisable(GL_BLEND);
		}
		else
		{
			glEnable(GL_BLEND);
			glBlendFunc(srcFactor, dstFactor);
		}
	}

	//
	// check depthmask
	//
	if (diff & GLS_DEPTHMASK)
	{
		if (stateBits & GLS_DEPTHMASK)
		{
			glDepthMask(GL_FALSE);
		}
		else
		{
			glDepthMask(GL_TRUE);
		}
	}

	//
	// check colormask
	//
	if (diff & (GLS_REDMASK | GLS_GREENMASK | GLS_BLUEMASK | GLS_ALPHAMASK))
	{
		GLboolean r = (stateBits & GLS_REDMASK) ? GL_FALSE : GL_TRUE;
		GLboolean g = (stateBits & GLS_GREENMASK) ? GL_FALSE : GL_TRUE;
		GLboolean b = (stateBits & GLS_BLUEMASK) ? GL_FALSE : GL_TRUE;
		GLboolean a = (stateBits & GLS_ALPHAMASK) ? GL_FALSE : GL_TRUE;
		glColorMask(r, g, b, a);
	}

	//
	// fill/line mode
	//
	if (diff & GLS_POLYMODE_LINE)
	{
		if (stateBits & GLS_POLYMODE_LINE)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	//
	// polygon offset
	//
	if (diff & GLS_POLYGON_OFFSET)
	{
		if (stateBits & GLS_POLYGON_OFFSET)
		{
			glPolygonOffset(_polyOfsScale, _polyOfsBias);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glEnable(GL_POLYGON_OFFSET_LINE);
		}
		else
		{
			glDisable(GL_POLYGON_OFFSET_FILL);
			glDisable(GL_POLYGON_OFFSET_LINE);
		}
	}

	//
	// stencil
	//
	if (diff & (GLS_STENCIL_FUNC_BITS | GLS_STENCIL_OP_BITS))
	{
		if ((stateBits & (GLS_STENCIL_FUNC_BITS | GLS_STENCIL_OP_BITS)) != 0)
		{
			glEnable(GL_STENCIL_TEST);
		}
		else
		{
			glDisable(GL_STENCIL_TEST);
		}
	}
	if (diff & (GLS_STENCIL_FUNC_BITS | GLS_STENCIL_FUNC_REF_BITS | GLS_STENCIL_FUNC_MASK_BITS))
	{
		GLuint ref = GLuint((stateBits & GLS_STENCIL_FUNC_REF_BITS) >> GLS_STENCIL_FUNC_REF_SHIFT);
		GLuint mask = GLuint((stateBits & GLS_STENCIL_FUNC_MASK_BITS) >> GLS_STENCIL_FUNC_MASK_SHIFT);
		GLenum func = 0;

		switch (stateBits & GLS_STENCIL_FUNC_BITS)
		{
		case GLS_STENCIL_FUNC_NEVER:
			func = GL_NEVER;
			break;
		case GLS_STENCIL_FUNC_LESS:
			func = GL_LESS;
			break;
		case GLS_STENCIL_FUNC_EQUAL:
			func = GL_EQUAL;
			break;
		case GLS_STENCIL_FUNC_LEQUAL:
			func = GL_LEQUAL;
			break;
		case GLS_STENCIL_FUNC_GREATER:
			func = GL_GREATER;
			break;
		case GLS_STENCIL_FUNC_NOTEQUAL:
			func = GL_NOTEQUAL;
			break;
		case GLS_STENCIL_FUNC_GEQUAL:
			func = GL_GEQUAL;
			break;
		case GLS_STENCIL_FUNC_ALWAYS:
			func = GL_ALWAYS;
			break;
		}
		glStencilFunc(func, ref, mask);
	}
	if (diff & (GLS_STENCIL_OP_FAIL_BITS | GLS_STENCIL_OP_ZFAIL_BITS | GLS_STENCIL_OP_PASS_BITS))
	{
		GLenum sFail = 0;
		GLenum zFail = 0;
		GLenum pass = 0;

		switch (stateBits & GLS_STENCIL_OP_FAIL_BITS)
		{
		case GLS_STENCIL_OP_FAIL_KEEP:
			sFail = GL_KEEP;
			break;
		case GLS_STENCIL_OP_FAIL_ZERO:
			sFail = GL_ZERO;
			break;
		case GLS_STENCIL_OP_FAIL_REPLACE:
			sFail = GL_REPLACE;
			break;
		case GLS_STENCIL_OP_FAIL_INCR:
			sFail = GL_INCR;
			break;
		case GLS_STENCIL_OP_FAIL_DECR:
			sFail = GL_DECR;
			break;
		case GLS_STENCIL_OP_FAIL_INVERT:
			sFail = GL_INVERT;
			break;
		case GLS_STENCIL_OP_FAIL_INCR_WRAP:
			sFail = GL_INCR_WRAP;
			break;
		case GLS_STENCIL_OP_FAIL_DECR_WRAP:
			sFail = GL_DECR_WRAP;
			break;
		}
		switch (stateBits & GLS_STENCIL_OP_ZFAIL_BITS)
		{
		case GLS_STENCIL_OP_ZFAIL_KEEP:
			zFail = GL_KEEP;
			break;
		case GLS_STENCIL_OP_ZFAIL_ZERO:
			zFail = GL_ZERO;
			break;
		case GLS_STENCIL_OP_ZFAIL_REPLACE:
			zFail = GL_REPLACE;
			break;
		case GLS_STENCIL_OP_ZFAIL_INCR:
			zFail = GL_INCR;
			break;
		case GLS_STENCIL_OP_ZFAIL_DECR:
			zFail = GL_DECR;
			break;
		case GLS_STENCIL_OP_ZFAIL_INVERT:
			zFail = GL_INVERT;
			break;
		case GLS_STENCIL_OP_ZFAIL_INCR_WRAP:
			zFail = GL_INCR_WRAP;
			break;
		case GLS_STENCIL_OP_ZFAIL_DECR_WRAP:
			zFail = GL_DECR_WRAP;
			break;
		}
		switch (stateBits & GLS_STENCIL_OP_PASS_BITS)
		{
		case GLS_STENCIL_OP_PASS_KEEP:
			pass = GL_KEEP;
			break;
		case GLS_STENCIL_OP_PASS_ZERO:
			pass = GL_ZERO;
			break;
		case GLS_STENCIL_OP_PASS_REPLACE:
			pass = GL_REPLACE;
			break;
		case GLS_STENCIL_OP_PASS_INCR:
			pass = GL_INCR;
			break;
		case GLS_STENCIL_OP_PASS_DECR:
			pass = GL_DECR;
			break;
		case GLS_STENCIL_OP_PASS_INVERT:
			pass = GL_INVERT;
			break;
		case GLS_STENCIL_OP_PASS_INCR_WRAP:
			pass = GL_INCR_WRAP;
			break;
		case GLS_STENCIL_OP_PASS_DECR_WRAP:
			pass = GL_DECR_WRAP;
			break;
		}
		glStencilOp(sFail, zFail, pass);
	}

	m_glStateBits = stateBits;
}

void Pipeline::setWorldPosition(const float3& v)
{
	m_worldPosition = v;
	m_bChangeWVP = true;
}

void Pipeline::setWorldScale(const float3& v)
{
	m_worldScale = v;
	m_bChangeWVP = true;
}

void Pipeline::setWorldEulerRotation(const float3& v)
{
	m_worldEulerAngles = v;

	const auto q_x = glm::angleAxis(v.x, glm::vec3(1, 0, 0));
	const auto q_y = glm::angleAxis(v.y, glm::vec3(0, 1, 0));
	const auto q_z = glm::angleAxis(v.z, glm::vec3(0, 0, 1));

	m_worldRotation = q_z * q_y * q_x;
	m_bChangeWVP = true;
}

void Pipeline::setWorldQuaternionRotation(const quat& v)
{
	m_worldRotation = v;
	m_bChangeWVP = true;
}

void Pipeline::setScreenRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
	g_misc.screen_w = w;
	g_misc.screen_h = h;
	g_misc.screen_x = x;
	g_misc.screen_y = y;
	glViewport(GLint(x), GLint(y), GLsizei(w), GLsizei(h));
	m_bChangeView = true;
}

void Pipeline::useProgram(GpuProgram& prog)
{
	if (m_activeProgram != prog.mProgId)
	{
		prog.use();
		m_activeProgram = prog.mProgId;
	}
}

void Pipeline::bindVertexBuffer(GpuBuffer& b, int index, uint32_t offset, uint32_t stride)
{
	if (index > -1)
	{
		assert(index < MAX_BUFFER_BINDING);

		if (m_activeVertexArrayBinding[index].buffer != b.mBuffer
			|| m_activeVertexArrayBinding[index].stride != stride
			|| m_activeVertexArrayBinding[index].offset != offset)
		{
			b.bindVertexBuffer(index, offset, stride);
			m_activeVertexArrayBinding[index].buffer = b.mBuffer;
			m_activeVertexArrayBinding[index].offset = offset;
			m_activeVertexArrayBinding[index].stride = stride;
		}
	}
	else
	{
		if (m_activeArrayBuffer != b.mBuffer)
		{
			b.bind();
			m_activeArrayBuffer = b.mBuffer;
		}
	}
}

void Pipeline::bindIndexBuffer(GpuBuffer& b)
{
	if (m_activeElementBuffer != b.mBuffer)
	{
		b.bind();
		m_activeElementBuffer = b.mBuffer;
	}
}

void Pipeline::bindUniformBuffer(GpuBuffer& b, int index, uint32_t offset, uint32_t size)
{
	if (m_activeUniformBinding[index].buffer != b.mBuffer
		|| m_activeUniformBinding[index].size != size
		|| m_activeUniformBinding[index].offset != offset)
	{
		b.bindIndexed(index, offset, size);
		m_activeUniformBinding[index].buffer = b.mBuffer;
		m_activeUniformBinding[index].offset = offset;
		m_activeUniformBinding[index].size = size;
	}
}

void Pipeline::drawArrays(eDrawMode mode, int first, uint32_t count)
{
	const GLenum mode_ = GL_castDrawMode(mode);

	GL_CHECK(glDrawArrays(mode_, first, count));
}

void Pipeline::drawElements(eDrawMode mode, uint32_t count, eDataType type, uint32_t offset)
{
	const GLenum mode_ = GL_castDrawMode(mode);
	const GLenum type_ = GL_castDataType(type);

	GL_CHECK(glDrawElements(mode_, count, type_, reinterpret_cast<void*>(offset)));
}

void Pipeline::drawElements(eDrawMode mode, uint32_t count, eDataType type, uint32_t offset, uint32_t baseVertex)
{
	const GLenum mode_ = GL_castDrawMode(mode);
	const GLenum type_ = GL_castDataType(type);

	GL_CHECK(glDrawElementsBaseVertex(mode_, count, type_, reinterpret_cast<void*>(offset), baseVertex));
}

void Pipeline::bindTexture(GpuTexture& tex, int unit)
{
	if (m_tmus[unit].target != tex.getApiTarget() || m_tmus[unit].texId != tex.mTexture)
	{
		tex.bind(unit);
		m_tmus[unit].target = tex.getApiTarget();
		m_tmus[unit].texId = tex.mTexture;
	}
}

#define INIT_CB(a,b) if (b)\
{\
	bindUniformBuffer(*b, a);\
	if (!b->isMapped()) b->mapPeristentWrite();\
}

void Pipeline::init()
{
	INIT_CB(CB_MATRIX, m_mtxBuffer);
	INIT_CB(CB_MISC, m_miscBuffer);
	INIT_CB(CB_SUN, m_sunBuffer);
	INIT_CB(CB_CAMERA, m_camBuffer);
}

#define UPDATE_CB(a,b) if (a) \
{\
	assert(a->isMapped());\
	::memcpy(a->mappedAddress(), &b, sizeof(b));\
}

void Pipeline::update(float time)
{
	g_misc_t* misc = reinterpret_cast<g_misc_t*>(m_miscBuffer->mappedAddress());

	g_misc.time = time;
	misc->time = time;

	if (m_bChangeWVP)
	{
		if (g_cam.yfov == 0.0f)
		{
			g_mtx.m_P = glm::ortho(-(g_misc.screen_x / 2.0f), (g_misc.screen_x / 2.0f), (g_misc.screen_y / 2.0f), -(g_misc.screen_y / 2.0f), g_cam.znear, g_cam.zfar);
		}
		else
		{
			g_mtx.m_P = glm::perspective(g_cam.yfov, float(g_misc.screen_x) / float(g_misc.screen_y), g_cam.znear, g_cam.zfar);
		}

		g_mtx.m_V = glm::lookAt(float3(g_cam.position), float3(g_cam.position + g_cam.direction), float3(g_cam.up));

		g_mtx.m_VP = g_mtx.m_P * g_mtx.m_V;

		mat4 trans = mat4(1.0f);
		trans = glm::translate(trans, m_worldPosition);
		trans = trans * mat4(m_worldRotation);
		trans = glm::scale(trans, m_worldScale);
		g_mtx.m_W = trans;

		g_mtx.m_VP = g_mtx.m_P * g_mtx.m_V;
		g_mtx.m_WVP = g_mtx.m_P * g_mtx.m_V * g_mtx.m_W;
		g_mtx.m_WV = g_mtx.m_V * g_mtx.m_W;

		g_mtx.m_iP = glm::inverse(g_mtx.m_P);
		g_mtx.m_iVP = glm::inverse(g_mtx.m_VP);

		//Normal = mat3(transpose(inverse(model))) * aNormal;
		g_mtx.m_Normal = mat4(mat3(glm::transpose(glm::inverse(g_mtx.m_W))));

		UPDATE_CB(m_mtxBuffer, g_mtx);
		UPDATE_CB(m_camBuffer, g_cam);

		m_bChangeWVP = false;
	}

	if (m_bChangeView)
	{
		UPDATE_CB(m_miscBuffer, g_misc);
		m_bChangeView = false;
	}
	if (m_bChangeSun)
	{
		UPDATE_CB(m_sunBuffer, g_sun);
		m_bChangeSun = false;
	}

}
