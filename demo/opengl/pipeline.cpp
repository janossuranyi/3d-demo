#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "pipeline.h"

Pipeline::Pipeline()
{
	g_misc.f_time = SDL_static_cast(float, SDL_GetTicks64());
	g_misc.i_screen_x = 1920;
	g_misc.i_screen_y = 1080;

	g_sun.v_position = glm::vec4(5, 5, -5, 1);
	g_sun.v_direction = glm::normalize(glm::vec4(0,0,0,1) - g_sun.v_position);
	g_sun.v_direction.w = 0.0f;
	g_sun.v_color = glm::vec4(1, 1, 1, 1);
	g_sun.f_intensity = 5.0f;

	g_cam.v_position = glm::vec4(0, 0, -5, 1);
	g_cam.v_direction = glm::normalize(glm::vec4(0,0,0,1) - g_cam.v_position);
	g_cam.v_direction.w = 0.0f;
	g_cam.v_up = glm::vec4(0, 1, 0, 0);
	g_cam.v_near_far_fov = glm::vec4(0.01, 100.0, 45.0f, 0.0f);

	m_activeArrayBuffer = 0;
	m_activeElementBuffer = 0;
	m_activeVertexArray = 0;
	m_activeFrameBuffer = 0;
	_polyOfsBias = 0.0f;
	_polyOfsScale = 0.0f;

	for (int i = 0; i < MAX_TEXTURE_UNITS; ++i)
	{
		m_tmus[i].target = 0;
		m_tmus[i].texId = 0;
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
	if (diff & (GLS_CULL_BITS))//| GLS_MIRROR_VIEW ) )
	{
		switch (stateBits & GLS_CULL_BITS)
		{
		case GLS_CULL_TWOSIDED:
			glDisable(GL_CULL_FACE);
			break;

		case GLS_CULL_BACKSIDED:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			break;

		case GLS_CULL_FRONTSIDED:
		default:
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

void Pipeline::setWorldPosition(const glm::vec3& v)
{
	m_worldPosition = v;
}

void Pipeline::setWorldScale(const glm::vec3& v)
{
	m_worldScale = v;
}

void Pipeline::setWorldEulerRotation(const glm::vec3& v)
{
	m_worldEulerAngles = v;

	const auto q_x = glm::angleAxis(v.x, glm::vec3(1, 0, 0));
	const auto q_y = glm::angleAxis(v.y, glm::vec3(0, 1, 0));
	const auto q_z = glm::angleAxis(v.z, glm::vec3(0, 0, 1));

	m_worldRotation = q_z * q_y * q_x;
}

void Pipeline::setWorldQuaternionRotation(const glm::quat& v)
{
	m_worldRotation = v;
}

void Pipeline::setScreenRect(unsigned width, unsigned height)
{
	g_misc.i_screen_x = width;
	g_misc.i_screen_y = height;
}

void Pipeline::bindVertexBuffer(GpuBuffer& b, int index)
{
	if (index > -1)
	{

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

void Pipeline::update(float time)
{
	g_misc.f_time = time;

	if (g_cam.v_near_far_fov.z == 0.0f)
	{
		g_mtx.m_P = glm::ortho(-(g_misc.i_screen_x / 2.0f), (g_misc.i_screen_x / 2.0f), (g_misc.i_screen_y / 2.0f), -(g_misc.i_screen_y / 2.0f), g_cam.v_near_far_fov.x, g_cam.v_near_far_fov.y);
	}
	else
	{
		g_mtx.m_P = glm::perspective(g_cam.v_near_far_fov.z, float(g_misc.i_screen_x) / float(g_misc.i_screen_y), g_cam.v_near_far_fov.x, g_cam.v_near_far_fov.y);
	}

	g_mtx.m_V = glm::lookAt(glm::vec3(g_cam.v_position), glm::vec3(g_cam.v_position + g_cam.v_direction), glm::vec3(g_cam.v_up));

	g_mtx.m_VP = g_mtx.m_P * g_mtx.m_V;
	
	glm::mat4 trans = glm::mat4(1.0f);
	trans = glm::translate(trans, m_worldPosition);
	trans = trans * glm::mat4(m_worldRotation);
	trans = glm::scale(trans, m_worldScale);
	g_mtx.m_W = trans;
	
	g_mtx.m_VP = g_mtx.m_P * g_mtx.m_V;
	g_mtx.m_WVP = g_mtx.m_P * g_mtx.m_V * g_mtx.m_W;
	g_mtx.m_WV = g_mtx.m_V * g_mtx.m_W;

	g_mtx.m_iP = glm::inverse(g_mtx.m_P);
	g_mtx.m_iVP = glm::inverse(g_mtx.m_VP);

	//Normal = mat3(transpose(inverse(model))) * aNormal;
	g_mtx.m_Normal = glm::mat4(glm::mat3(glm::transpose(glm::inverse(g_mtx.m_W))));

}
