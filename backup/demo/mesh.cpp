#include <string>
#include <cstring>
#include <tiny_gltf.h>
#include "logger.h"
#include "mesh.h"
#include "heap.h"
#include "gpu_buffer.h"
#include "pipeline.h"

//using namespace tinygltf;
#define _TG tinygltf::

bool Mesh3D::loadFromGLTF(const char* filename, int meshIdx, int primitiveIdx)
{

    _TG Model model;
    _TG TinyGLTF loader;

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

    _TG Mesh& m = model.meshes[meshIdx];
    if (m.primitives.size() < primitiveIdx)
    {
        Error("%s, line %d: primitive index out of range", __FILE__, __LINE__);
        return false;
    }    

    return importFromGLTF(model, m.primitives[primitiveIdx]);
}

bool Mesh3D::importFromGLTF(const tinygltf::Model& model, const tinygltf::Primitive& meshPrimitive)
{
    for (auto& p : meshPrimitive.attributes)
    {
        const _TG Accessor& access = model.accessors[p.second];
        const _TG BufferView& view = model.bufferViews[access.bufferView];
        const _TG Buffer& buffer = model.buffers[view.buffer];
        const unsigned char* first = buffer.data.data() + view.byteOffset + access.byteOffset;

        const size_t byteStride = view.byteStride;

        if (p.first == "POSITION")
        {
            // allocate position memory
            VertexAttribute attr{ "POSITION", ComponentType::FLOAT, 3, access.count, access.normalized, 0, byteStride, 0, view.byteLength };
            m_Position_layout = attr;

            m_Positions.assign(first, first + view.byteLength);

            // bounds
            m_Bounds[0].x = static_cast<float>(access.minValues[0]);
            m_Bounds[0].y = static_cast<float>(access.minValues[1]);
            m_Bounds[0].z = static_cast<float>(access.minValues[2]);
            m_Bounds[1].x = static_cast<float>(access.maxValues[0]);
            m_Bounds[1].y = static_cast<float>(access.maxValues[1]);
            m_Bounds[1].z = static_cast<float>(access.maxValues[2]);

        }
        else if (p.first == "NORMAL")
        {
            VertexAttribute attr{ "NORMAL", ComponentType::FLOAT, 3, access.count, access.normalized, 0, byteStride, 0, view.byteLength };
            m_Normal_layout = attr;

            m_Normals.assign(first, first + view.byteLength);
        }
        else if (p.first == "TANGENT")
        {
            VertexAttribute attr{ "TANGENT", ComponentType::FLOAT, 4, access.count, access.normalized, 0, byteStride, 0, view.byteLength };
            m_Tangent_layout = attr;
            m_Tangents.assign(first, first + view.byteLength);
        }
        else if (p.first == "TEXCOORD_0")
        {
            VertexAttribute attr;
            attr.name = "TEXCOORD_0";
            attr.byteSize = view.byteLength;
            attr.normalized = access.normalized;
            attr.stride = byteStride;
            switch (access.componentType)
            {
            case TINYGLTF_COMPONENT_TYPE_FLOAT:
                attr.type = ComponentType::FLOAT;
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                attr.type = ComponentType::UNSIGNED_BYTE;
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                attr.type = ComponentType::UNSIGNED_SHORT;
                break;
            }
            attr.size = 2;
            attr.count = access.count;
            attr.offset = 0;

            m_TexCoord_layout = attr;
            m_TexCoords.assign(first, first + view.byteLength);
        }
        else if (p.first == "COLOR_0")
        {
            VertexAttribute attr;
            attr.name = "COLOR_0";
            attr.stride = byteStride;
            switch (access.componentType)
            {
            case TINYGLTF_COMPONENT_TYPE_FLOAT:
                attr.type = ComponentType::FLOAT;
                attr.normalized = false;
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                attr.type = ComponentType::UNSIGNED_BYTE;
                attr.normalized = true;
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                attr.type = ComponentType::UNSIGNED_SHORT;
                attr.normalized = true;
                break;
            }
            attr.size = access.type == TINYGLTF_TYPE_VEC3 ? 3 : 4;
            attr.count = access.count;
            attr.offset = 0;
            attr.byteSize = view.byteLength;

            m_Color_layout = attr;
            m_Colors.assign(first, first + view.byteLength);

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
    const _TG Accessor& access = model.accessors[meshPrimitive.indices];
    const _TG BufferView& view = model.bufferViews[access.bufferView];
    const _TG Buffer& buffer = model.buffers[view.buffer];
    const unsigned char* first = buffer.data.data() + view.byteOffset + access.byteOffset;

    switch (access.componentType)
    {
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        m_IndexType = ComponentType::UNSIGNED_SHORT;
        break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        m_IndexType = ComponentType::UNSIGNED_INT32;
        break;
    }

    m_NumIndex = access.count;

    m_Indices.assign(first, first + view.byteLength);
    m_material = meshPrimitive.material;

    Info("***** Mesh %d, material: %d/%d", m_id, m_material, meshPrimitive.material);

    return true;
}

int Mesh3D::material() const
{
    return m_material;
}

void Mesh3D::setMaterial(const int material)
{
    m_material = material;
}
