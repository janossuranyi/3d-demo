#include <string>
#include <cstring>
#include <tiny_gltf.h>
#include "logger.h"
#include "mesh.h"
#include "heap.h"
#include "gpu_buffer.h"
#include "pipeline.h"
using namespace tinygltf;

bool Mesh3D::loadFromGLTF(const char* filename, int meshIdx, int primitiveIdx)
{

    Model model;
    TinyGLTF loader;

    std::string warn, err;

    // try binary first
    if (!loader.LoadBinaryFromFile(&model, &err, &warn, filename))
    {
        // then ascii
        if (!loader.LoadASCIIFromFile(&model, &err, &warn, filename))
        {
            Error("%s, line %d: %s,%s", __FILE__, __LINE__, err.c_str(), warn.c_str());
            return false;
        }
    }

    if (model.meshes.size() < meshIdx)
    {
        Error("%s, line %d: mesh index out of range", __FILE__, __LINE__);
        return false;
    }

    Mesh& m = model.meshes[meshIdx];
    if (m.primitives.size() < primitiveIdx)
    {
        Error("%s, line %d: primitive index out of range", __FILE__, __LINE__);
        return false;
    }    

    return importFromGLTF(model, m.primitives[primitiveIdx]);
}

bool Mesh3D::importFromGLTF(const tinygltf::Model& model, const tinygltf::Primitive& meshPrimitive)
{
    for (auto p : meshPrimitive.attributes)
    {
        const Accessor& access = model.accessors[p.second];
        const BufferView& view = model.bufferViews[access.bufferView];
        const Buffer& buffer = model.buffers[view.buffer];

        if (p.first == "POSITION")
        {
            // allocate position memory
            VertexAttribute attr{ "POSITION", eDataType::FLOAT, 3, access.count, false, 0, 0, 0 };
            m_Position_layout = attr;
            m_Positions = Mem_Alloc16(view.byteLength);
            ::memcpy(m_Positions, buffer.data.data() + view.byteOffset + access.byteOffset, view.byteLength);

            // bounds
            m_Bounds[0].x = access.minValues[0];
            m_Bounds[0].y = access.minValues[1];
            m_Bounds[0].z = access.minValues[2];
            m_Bounds[1].x = access.maxValues[0];
            m_Bounds[1].y = access.maxValues[1];
            m_Bounds[1].z = access.maxValues[2];

        }
        else if (p.first == "NORMAL")
        {
            VertexAttribute attr{ "NORMAL", eDataType::FLOAT, 3, access.count, false, 0, 0, 0, view.byteLength };
            m_Normal_layout = attr;
            m_Normals = Mem_Alloc16(view.byteLength);
            ::memcpy(m_Normals, buffer.data.data() + view.byteOffset + access.byteOffset, view.byteLength);
        }
        else if (p.first == "TANGENT")
        {
            VertexAttribute attr{ "TANGENT", eDataType::FLOAT, 4, access.count, false, 0, 0, 0, view.byteLength };
            m_Tangent_layout = attr;
            m_Tangents = Mem_Alloc16(view.byteLength);
            ::memcpy(m_Tangents, buffer.data.data() + view.byteOffset + access.byteOffset, view.byteLength);
        }
        else if (p.first == "TEXCOORD_0")
        {
            VertexAttribute attr;
            attr.name = "TEXCOORD_0";
            attr.byteSize = view.byteLength;
            switch (access.componentType)
            {
            case TINYGLTF_COMPONENT_TYPE_FLOAT:
                attr.type = eDataType::FLOAT;
                attr.normalized = false;
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                attr.type = eDataType::UNSIGNED_BYTE;
                attr.normalized = true;
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                attr.type = eDataType::UNSIGNED_SHORT;
                attr.normalized = true;
                break;
            }
            attr.size = 2;
            attr.count = access.count;
            attr.stride = 0;
            attr.offset = 0;

            m_TexCoord_layout = attr;
            m_TexCoords = Mem_Alloc16(view.byteLength);
            ::memcpy(m_TexCoords, buffer.data.data() + view.byteOffset + access.byteOffset, view.byteLength);
        }
        else if (p.first == "COLOR_0")
        {
            VertexAttribute attr;
            attr.name = "COLOR_0";
            switch (access.componentType)
            {
            case TINYGLTF_COMPONENT_TYPE_FLOAT:
                attr.type = eDataType::FLOAT;
                attr.normalized = false;
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                attr.type = eDataType::UNSIGNED_BYTE;
                attr.normalized = true;
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                attr.type = eDataType::UNSIGNED_SHORT;
                attr.normalized = true;
                break;
            }
            attr.size = access.type == TINYGLTF_TYPE_VEC3 ? 3 : 4;
            attr.count = access.count;
            attr.stride = 0;
            attr.offset = 0;
            attr.byteSize = view.byteLength;

            m_Color_layout = attr;
            m_Colors = Mem_Alloc16(view.byteLength);
            ::memcpy(m_Colors, buffer.data.data() + view.byteOffset + access.byteOffset, view.byteLength);
        }
    }

    // index

    switch (meshPrimitive.mode)
    {
    case TINYGLTF_MODE_LINE:
        m_Mode = eDrawMode::LINES;
        break;
    case TINYGLTF_MODE_LINE_LOOP:
        m_Mode = eDrawMode::LINE_LOOP;
        break;
    case TINYGLTF_MODE_LINE_STRIP:
        m_Mode = eDrawMode::LINE_STRIP;
        break;
    case TINYGLTF_MODE_POINTS:
        m_Mode = eDrawMode::POINTS;
        break;
    case TINYGLTF_MODE_TRIANGLES:
        m_Mode = eDrawMode::TRIANGLES;
        break;
    case TINYGLTF_MODE_TRIANGLE_FAN:
        m_Mode = eDrawMode::TRIANGLE_FAN;
        break;
    case TINYGLTF_MODE_TRIANGLE_STRIP:
        m_Mode = eDrawMode::TRIANGLE_STRIP;
        break;
    }
    const Accessor& access = model.accessors[meshPrimitive.indices];
    const BufferView& view = model.bufferViews[access.bufferView];
    const Buffer& buffer = model.buffers[view.buffer];

    switch (access.componentType)
    {
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        m_IndexType = eDataType::UNSIGNED_SHORT;
        break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        m_IndexType = eDataType::UNSIGNED_INT32;
        break;
    }

    m_NumIndex = access.count;

    m_Indices = Mem_Alloc16(view.byteLength);
    ::memcpy(m_Indices, buffer.data.data() + view.byteOffset + access.byteOffset, view.byteLength);

    return true;
}

void RenderMesh3D::compile(const Mesh3D& mesh)
{
    if (isCompiled())
        return;

    int index = 0;
    m_Layout.begin();
    if (mesh.getPositionLayout().count)
    {
        const VertexAttribute& va = mesh.getPositionLayout();
        m_PositionBuf.create(va.byteSize, eGpuBufferUsage::STATIC, mesh.getPositions());
        m_Layout.with(index++, va.size, va.type, va.normalized, 0, 0, &m_PositionBuf);
    }
    if (mesh.getNormalLayout().count)
    {
        const VertexAttribute& va = mesh.getNormalLayout();
        m_NormalBuf.create(va.byteSize, eGpuBufferUsage::STATIC, mesh.getNormals());
        m_Layout.with(index++, va.size, va.type, va.normalized, 0, 0, &m_NormalBuf);
    }
    if (mesh.getTangentLayout().count)
    {
        const VertexAttribute& va = mesh.getTangentLayout();
        m_TangentBuf.create(va.byteSize, eGpuBufferUsage::STATIC, mesh.getTangents());
        m_Layout.with(index++, va.size, va.type, va.normalized, 0, 0, &m_TangentBuf);
    }
    if (mesh.getTexCoordLayout().count)
    {
        const VertexAttribute& va = mesh.getTexCoordLayout();
        m_TexCoordBuf.create(va.byteSize, eGpuBufferUsage::STATIC, mesh.getTexCoords());
        m_Layout.with(index++, va.size, va.type, va.normalized, 0, 0, &m_TexCoordBuf);
    }
    if (mesh.getColorLayout().count)
    {
        const VertexAttribute& va = mesh.getColorLayout();
        m_ColorBuf.create(va.byteSize, eGpuBufferUsage::STATIC, mesh.getColors());
        m_Layout.with(index++, va.size, va.type, va.normalized, 0, 0, &m_ColorBuf);
    }
    m_Layout.end();

    if (mesh.getNumIndex())
    {
        m_IndexBuf.create(mesh.getNumIndex() * (mesh.getIndexType() == eDataType::UNSIGNED_SHORT ? 2 : 4), eGpuBufferUsage::STATIC, mesh.getIndices());
        m_NumIndex = mesh.getNumIndex();
        m_IndexType = mesh.getIndexType();
    }
    m_Mode = mesh.getDrawMode();

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
