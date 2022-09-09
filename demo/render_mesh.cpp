#include "mesh.h"
#include "world.h"
#include "pipeline.h"

void RenderMesh3D::compile(const Mesh3D& mesh)
{
    if (isCompiled())
        return;

    int index = 0;
    m_NumVertices = mesh.positionLayout().count;

    m_Layout.begin();
    if (mesh.positionLayout().count)
    {
        const VertexAttribute& va = mesh.positionLayout();
        m_PositionBuf.create(va.byteSize, eGpuBufferUsage::STATIC, 0, mesh.positions().data());
        m_Layout.with(index++, va.size, va.type, va.normalized, 0, 0, &m_PositionBuf);
    }
    if (mesh.normalLayout().count)
    {
        const VertexAttribute& va = mesh.normalLayout();
        m_NormalBuf.create(va.byteSize, eGpuBufferUsage::STATIC, 0, mesh.normals().data());
        m_Layout.with(index++, va.size, va.type, va.normalized, 0, 0, &m_NormalBuf);
    }
    if (mesh.tangentLayout().count)
    {
        const VertexAttribute& va = mesh.tangentLayout();
        m_TangentBuf.create(va.byteSize, eGpuBufferUsage::STATIC, 0, mesh.tangents().data());
        m_Layout.with(index++, va.size, va.type, va.normalized, 0, 0, &m_TangentBuf);
    }
    if (mesh.texCoordLayout().count)
    {
        const VertexAttribute& va = mesh.texCoordLayout();
        m_TexCoordBuf.create(va.byteSize, eGpuBufferUsage::STATIC, 0, mesh.texCoords().data());
        m_Layout.with(index++, va.size, va.type, va.normalized, 0, 0, &m_TexCoordBuf);
    }
    if (mesh.colorLayout().count)
    {
        const VertexAttribute& va = mesh.colorLayout();
        m_ColorBuf.create(va.byteSize, eGpuBufferUsage::STATIC, 0, mesh.colors().data());
        m_Layout.with(index++, va.size, va.type, va.normalized, 0, 0, &m_ColorBuf);
    }
    m_Layout.end();

    if (mesh.numIndex())
    {
        m_IndexBuf.create(mesh.numIndex() * (mesh.indexType() == eDataType::UNSIGNED_SHORT ? 2 : 4), eGpuBufferUsage::STATIC, 0, mesh.indices().data());
        m_NumIndex = mesh.numIndex();
        m_IndexType = mesh.indexType();
    }
    m_Mode = mesh.drawMode();
    mesh.bounds(m_Min, m_Max);

    m_bCompiled = true;
}

void RenderMesh3D::render(Pipeline& p) const
{
    p.setLayout(m_Layout);
    p.setMaterial(m_material);
    p.update();

    if (m_IndexBuf.isCreated())
    {
        p.bindIndexBuffer(m_IndexBuf);
        p.drawElements(m_Mode, m_NumIndex, m_IndexType, 0U);
    }
    else
    {
        p.drawArrays(m_Mode, 0, m_Layout.m_numAttribs);
    }
}

int RenderMesh3D::material() const
{
    return m_material;
}

void RenderMesh3D::setMaterial(const int material)
{
    m_material = material;
}
