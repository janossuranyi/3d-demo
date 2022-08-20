
#include "gpu_utils.h"
#include "gpu_vertex_layout.h"

VertexLayout::~VertexLayout()
{
    if (m_vao)
        GL_CHECK(glDeleteVertexArrays(1, &m_vao));
}

VertexLayout& VertexLayout::create()
{
    if (!m_vao) GL_CHECK(glGenVertexArrays(1, &m_vao));

    GL_CHECK(glBindVertexArray(m_vao));

    return *this;
}

VertexLayout& VertexLayout::with(unsigned int index, unsigned int size, eDataType type, bool normalized, unsigned int offset, unsigned int stream)
{
    glVertexAttribFormat(index, size, GL_castDataType(type), normalized, offset);
    glVertexAttribBinding(index, stream);
}
