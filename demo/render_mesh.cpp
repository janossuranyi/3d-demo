#include "mesh.h"
#include "pipeline.h"

void RenderMesh3D::compile(const Mesh3D& mesh)
{
    if (isCompiled())
        return;

    int index = 0;
    m_Layout.begin();
    if (mesh.getPositionLayout().count)
    {
        const VertexAttribute& va = mesh.getPositionLayout();
        m_PositionBuf.create(va.byteSize, eGpuBufferUsage::STATIC, 0, mesh.getPositions());
        m_Layout.with(index++, va.size, va.type, va.normalized, 0, 0, &m_PositionBuf);
    }
    if (mesh.getNormalLayout().count)
    {
        const VertexAttribute& va = mesh.getNormalLayout();
        m_NormalBuf.create(va.byteSize, eGpuBufferUsage::STATIC, 0, mesh.getNormals());
        m_Layout.with(index++, va.size, va.type, va.normalized, 0, 0, &m_NormalBuf);
    }
    if (mesh.getTangentLayout().count)
    {
        const VertexAttribute& va = mesh.getTangentLayout();
        m_TangentBuf.create(va.byteSize, eGpuBufferUsage::STATIC, 0, mesh.getTangents());
        m_Layout.with(index++, va.size, va.type, va.normalized, 0, 0, &m_TangentBuf);
    }
    if (mesh.getTexCoordLayout().count)
    {
        const VertexAttribute& va = mesh.getTexCoordLayout();
        m_TexCoordBuf.create(va.byteSize, eGpuBufferUsage::STATIC, 0, mesh.getTexCoords());
        m_Layout.with(index++, va.size, va.type, va.normalized, 0, 0, &m_TexCoordBuf);
    }
    if (mesh.getColorLayout().count)
    {
        const VertexAttribute& va = mesh.getColorLayout();
        m_ColorBuf.create(va.byteSize, eGpuBufferUsage::STATIC, 0, mesh.getColors());
        m_Layout.with(index++, va.size, va.type, va.normalized, 0, 0, &m_ColorBuf);
    }
    m_Layout.end();

    if (mesh.getNumIndex())
    {
        m_IndexBuf.create(mesh.getNumIndex() * (mesh.getIndexType() == eDataType::UNSIGNED_SHORT ? 2 : 4), eGpuBufferUsage::STATIC, 0, mesh.getIndices());
        m_NumIndex = mesh.getNumIndex();
        m_IndexType = mesh.getIndexType();
    }
    m_Mode = mesh.getDrawMode();
    mesh.getBounds(m_Min, m_Max);

    m_bCompiled = true;
}

void RenderMesh3D::render(Pipeline& p) const
{
    m_Layout.bind();

    if (m_IndexBuf.isCreated())
    {
        m_IndexBuf.bind();
        p.drawElements(m_Mode, m_NumIndex, m_IndexType, 0U);
    }
}
