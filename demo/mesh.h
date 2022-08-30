#pragma once

#include <memory>
#include <vector>
#include <cinttypes>
#include <tiny_gltf.h>
#include "filesystem.h"
#include "logger.h"
#include "types.h"
#include "material.h"
#include "gpu_types.h"
#include "gpu_vertex_layout.h"
#include "gpu_buffer.h"

class World;

class Mesh3D
{
public:
	using Ptr = std::shared_ptr<Mesh3D>;

	Mesh3D() :
		m_Positions(),
		m_TexCoords(),
		m_Normals(),
		m_Tangents(),
		m_Colors(),
		m_Position_layout(),
		m_TexCoord_layout(),
		m_Normal_layout(),
		m_Tangent_layout(),
		m_Color_layout(),
		m_Indices(),
		m_IndexType(eDataType::UNSIGNED_SHORT),
		m_NumIndex(),
		m_material(-1),
		m_Mode(),
		m_id(-1),
		m_Bounds() {}

	int id() const { return m_id; }
	void setId(int id) { m_id = id; }
	bool loadFromGLTF(const char* filename, int meshIdx, int primitiveIdx);
	bool importFromGLTF(const tinygltf::Model& model, const tinygltf::Primitive& meshPrimitive);

	int material() const;
	void setMaterial(const int);
	const void* positions() const { return m_Positions; }
	const void* texCoords() const { return m_TexCoords; }
	const void* normals() const { return m_Normals; }
	const void* tangents() const { return m_Tangents; }
	const void* colors() const { return m_Colors; }
	const void* indices() const { return m_Indices; }
	unsigned int numIndex() const { return m_NumIndex; }
	eDataType indexType() const { return m_IndexType; }
	eDrawMode drawMode() const { return m_Mode; }
	const VertexAttribute& positionLayout() const { return m_Position_layout; }
	const VertexAttribute& texCoordLayout() const { return m_TexCoord_layout; }
	const VertexAttribute& normalLayout() const { return m_Normal_layout; }
	const VertexAttribute& tangentLayout() const { return m_Tangent_layout; }
	const VertexAttribute& colorLayout() const { return m_Color_layout; }
	void bounds(vec3& min, vec3& max) const
	{
		min.x = m_Bounds[0].x;
		min.y = m_Bounds[0].y;
		min.z = m_Bounds[0].z;
		max.x = m_Bounds[1].x;
		max.y = m_Bounds[1].y;
		max.z = m_Bounds[1].z;
	}
private:

	int m_id;
	void* m_Positions;
	void* m_TexCoords;
	void* m_Normals;
	void* m_Tangents;
	void* m_Colors;
	void* m_Indices;

	VertexAttribute m_Position_layout;
	VertexAttribute m_TexCoord_layout;
	VertexAttribute m_Normal_layout;
	VertexAttribute m_Tangent_layout;
	VertexAttribute m_Color_layout;
	unsigned int m_NumIndex;
	
	int m_material;

	eDataType m_IndexType;
	eDrawMode m_Mode;

	glm::vec3 m_Bounds[2];

	/*
	* TODO
	* textures, materials, animations,...
	*/
};

class RenderMesh3D
{
public:
	using Ptr = std::shared_ptr<RenderMesh3D>;

	RenderMesh3D() :
		m_Mode(eDrawMode::TRIANGLES),
		m_PositionBuf(eGpuBufferTarget::VERTEX),
		m_TexCoordBuf(eGpuBufferTarget::VERTEX),
		m_NormalBuf(eGpuBufferTarget::VERTEX),
		m_TangentBuf(eGpuBufferTarget::VERTEX),
		m_ColorBuf(eGpuBufferTarget::VERTEX),
		m_IndexBuf(eGpuBufferTarget::INDEX),
		m_bCompiled(),
		m_NumIndex(),
		m_IndexType(),
		m_NumVertices(),
		m_material(-1),
		m_id(-1),
		m_Min(),
		m_Max() {}

	void compile(const Mesh3D& mesh);
	void render(Pipeline& p, World& world) const;

	int material() const;
	void setMaterial(const int);

	bool isCompiled() const;
	int id() const;
	void setId(int id);
private:
	int m_id;

	GpuBuffer m_PositionBuf;
	GpuBuffer m_TexCoordBuf;
	GpuBuffer m_NormalBuf;
	GpuBuffer m_TangentBuf;
	GpuBuffer m_ColorBuf;
	GpuBuffer m_IndexBuf;
	int m_material;

	VertexLayout m_Layout;
	vec3 m_Min, m_Max;

	unsigned int m_NumIndex;
	unsigned int m_NumVertices;

	eDataType m_IndexType;
	eDrawMode m_Mode;

	bool m_bCompiled;

};

inline int RenderMesh3D::id() const
{
	return m_id;
}
inline void RenderMesh3D::setId(int id)
{
	m_id = id;
}
inline bool RenderMesh3D::isCompiled() const
{
	return m_bCompiled;
}