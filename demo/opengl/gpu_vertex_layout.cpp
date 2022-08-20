
#include "gpu_utils.h"
#include "gpu_vertex_layout.h"

VertexLayout::~VertexLayout()
{
    if (m_vao)
        GL_CHECK(glDeleteVertexArrays(1, &m_vao));
}

VertexLayout& VertexLayout::begin()
{
    if (!m_vao) GL_CHECK(glGenVertexArrays(1, &m_vao));

    GL_CHECK(glBindVertexArray(m_vao));
    m_numAttribs = 0;

    return *this;
}

VertexLayout& VertexLayout::with(unsigned int index, unsigned int size, eDataType type, bool normalized, unsigned int offset, unsigned int stream)
{
    glEnableVertexAttribArray(index);
    glVertexAttribFormat(index, size, GL_castDataType(type), normalized, offset);
    glVertexAttribBinding(index, stream);
    ++m_numAttribs;

    return *this;
}

void VertexLayout::end() const
{
    GL_CHECK(glBindVertexArray(0));
    for (int i = 0; i < m_numAttribs; ++i)
    {
        glDisableVertexAttribArray(i);
    }
}

void VertexLayout::bind() const
{
    if (m_vao)
        GL_CHECK(glBindVertexArray(m_vao));
}
