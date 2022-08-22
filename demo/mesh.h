#pragma once

#include <memory>
#include <vector>
#include <cinttypes>
#include <glm/glm.hpp>
#include <tiny_gltf.h>
#include "filesystem.h"
#include "logger.h"
#include "gpu_types.h"
#include "gpu_vertex_layout.h"
#include "gpu_buffer.h"

class Mesh3D
{
public:
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
		m_Mode(),
		m_Bounds() {}

	using Ptr = std::shared_ptr<Mesh3D>;

	bool loadFromGLTF(const char* filename, int meshIdx, int primitiveIdx);
	bool importFromGLTF(const tinygltf::Model& model, const tinygltf::Primitive& meshPrimitive);

	const void* getPositions() const { return m_Positions; }
	const void* getTexCoords() const { return m_TexCoords; }
	const void* getNormals() const { return m_Normals; }
	const void* getTangents() const { return m_Tangents; }
	const void* getColors() const { return m_Colors; }
	const void* getIndices() const { return m_Indices; }
	unsigned int getNumIndex() const { return m_NumIndex; }
	eDataType getIndexType() const { return m_IndexType; }
	eDrawMode getDrawMode() const { return m_Mode; }
	const VertexAttribute& getPositionLayout() const { return m_Position_layout; }
	const VertexAttribute& getTexCoordLayout() const { return m_TexCoord_layout; }
	const VertexAttribute& getNormalLayout() const { return m_Normal_layout; }
	const VertexAttribute& getTangentLayout() const { return m_Tangent_layout; }
	const VertexAttribute& getColorLayout() const { return m_Color_layout; }
	void getBounds(glm::vec3& min, glm::vec3& max)
	{
		min.x = m_Bounds[0].x;
		min.y = m_Bounds[0].y;
		min.z = m_Bounds[0].z;
		max.x = m_Bounds[1].x;
		max.y = m_Bounds[1].y;
		max.z = m_Bounds[1].z;
	}
private:

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
	RenderMesh3D() :
		m_PositionBuf(eGpuBufferTarget::VERTEX),
		m_TexCoordBuf(eGpuBufferTarget::VERTEX),
		m_NormalBuf(eGpuBufferTarget::VERTEX),
		m_TangentBuf(eGpuBufferTarget::VERTEX),
		m_ColorBuf(eGpuBufferTarget::VERTEX),
		m_IndexBuf(eGpuBufferTarget::INDEX),
		m_bCompiled(),
		m_NumIndex(),
		m_IndexType(),
		m_Min(),
		m_Max() {}

	void compile(const Mesh3D& mesh);
	void render(Pipeline&) const;

	inline bool isCompiled() const { return m_bCompiled; }
private:
	GpuBuffer m_PositionBuf;
	GpuBuffer m_TexCoordBuf;
	GpuBuffer m_NormalBuf;
	GpuBuffer m_TangentBuf;
	GpuBuffer m_ColorBuf;
	GpuBuffer m_IndexBuf;

	VertexLayout m_Layout;
	glm::vec3 m_Min, m_Max;

	unsigned int m_NumIndex;
	eDataType m_IndexType;
	eDrawMode m_Mode;

	bool m_bCompiled;

};
