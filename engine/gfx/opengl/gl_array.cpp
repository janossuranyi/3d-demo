#include "gfx/opengl/gl_context.h"
#include "gfx/opengl/gl_helper.h"
#include "logger.h"

namespace gfx {

	void OpenGLRenderContext::operator()(const cmd::DeleteVertexLayout& cmd)
	{
		auto& vao = vertex_array_map_.find(cmd.handle);
		if (vao == std::end(vertex_array_map_))
		{
			return;
		}

		if (active_vertex_layout_ == cmd.handle)
		{
			GL_CHECK(glBindVertexArray(shared_vertex_array_));
			active_vertex_layout_ = VertexLayoutHandle{};
		}
		GL_CHECK(glDeleteVertexArrays(1, &vao->second));

		vertex_array_map_.erase(cmd.handle);
	}

	void OpenGLRenderContext::operator()(const cmd::CreateVertexLayout& cmd)
	{
		if (cmd.decl.empty() || vertex_array_map_.count(cmd.handle) > 0 || !GLEW_VERSION_4_3) {
			return;
		}

		GLuint vao;
		GL_CHECK(glGenVertexArrays(1, &vao));
		GL_CHECK(glBindVertexArray(vao));
		for (uint i = 0; i < cmd.decl.size(); ++i)
		{
			const auto& attr = cmd.decl.attributes()[i];
			GLenum type = MapAttribType(attr.type);
			GL_CHECK(glEnableVertexAttribArray(i));
			GL_CHECK(glVertexAttribFormat(i, attr.count, type, attr.normalized ? GL_TRUE : GL_FALSE, attr.offset));
			GL_CHECK(glVertexAttribBinding(i, attr.binding));
		}

		GL_CHECK(glBindVertexArray(shared_vertex_array_));
		active_vertex_layout_ = VertexLayoutHandle{};
		for (uint i = 0; i < cmd.decl.size(); ++i)
		{
			GL_CHECK(glDisableVertexAttribArray(i));
		}

		vertex_array_map_.emplace(cmd.handle, vao);
	}

}