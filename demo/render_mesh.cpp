#include "mesh.h"
#include "world.h"
#include "pipeline.h"

void RenderMesh3D::compile(const Mesh3D& mesh)
{
    if (isCompiled())
        return;

    size_t required_buffer_size{ 0 };

    required_buffer_size += mesh.positionLayout().byteSize;
    required_buffer_size += mesh.normalLayout().byteSize;
    required_buffer_size += mesh.tangentLayout().byteSize;
    required_buffer_size += mesh.texCoordLayout().byteSize;
    required_buffer_size += mesh.colorLayout().byteSize;

    m_VertexBuf.reset(new GpuBuffer(GpuBuffer::Target::VERTEX));
    m_VertexBuf->create(uint(required_buffer_size), GpuBuffer::Usage::STATIC, 0);

    int index = 0;
    size_t offset = 0;

    m_NumVertices = mesh.positionLayout().count;

    m_Layout.begin();
    if (mesh.positionLayout().count)
    {
        const VertexAttribute& va = mesh.positionLayout();
        m_VertexBuf->updateSubData(offset, va.byteSize, mesh.positions().data());
        m_Layout.with(index++, va.size, va.type, va.normalized, uint(offset), 0, m_VertexBuf.get());
        offset += va.byteSize;
    }
    if (mesh.normalLayout().count)
    {
        const VertexAttribute& va = mesh.normalLayout();
        m_VertexBuf->updateSubData(offset, va.byteSize, mesh.normals().data());
        m_Layout.with(index++, va.size, va.type, va.normalized, uint(offset), 0, m_VertexBuf.get());
        offset += va.byteSize;
    }
    if (mesh.tangentLayout().count)
    {
        const VertexAttribute& va = mesh.tangentLayout();
        m_VertexBuf->updateSubData(offset, va.byteSize, mesh.tangents().data());
        m_Layout.with(index++, va.size, va.type, va.normalized, uint(offset), 0, m_VertexBuf.get());
        offset += va.byteSize;
    }
    if (mesh.texCoordLayout().count)
    {
        const VertexAttribute& va = mesh.texCoordLayout();
        m_VertexBuf->updateSubData(offset, va.byteSize, mesh.texCoords().data());
        m_Layout.with(index++, va.size, va.type, va.normalized, uint(offset), 0, m_VertexBuf.get());
        offset += va.byteSize;
    }
    if (mesh.colorLayout().count)
    {
        const VertexAttribute& va = mesh.colorLayout();
        m_VertexBuf->updateSubData(offset, va.byteSize, mesh.colors().data());
        m_Layout.with(index++, va.size, va.type, va.normalized, uint(offset), 0, m_VertexBuf.get());
    }
    m_Layout.end();

    if (mesh.numIndex())
    {
        m_IndexBuf.reset(new GpuBuffer(GpuBuffer::Target::INDEX));
        m_IndexBuf->create(mesh.numIndex() * (mesh.indexType() == eDataType::UNSIGNED_SHORT ? 2 : 4), GpuBuffer::Usage::STATIC, 0, mesh.indices().data());
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

    if (m_IndexBuf->isCreated())
    {
        p.bindIndexBuffer(*m_IndexBuf.get());
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
