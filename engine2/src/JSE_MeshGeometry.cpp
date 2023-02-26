#include "JSE.h"

namespace js {
	MeshGeometry::MeshGeometry() :
		vertexCount_(0),
		indexCount_(0),
		material_(),
		sphereBV_(),
		boxBV_(),
		localTransform_(1.f),
		vertexBuffer_()
	{
	}

	const JsVector<vec3>& MeshGeometry::faceNormals() const
	{
		return faceNormals_;
	}

	const VertexBuffer& MeshGeometry::vertexBuffer() const
	{
		return vertexBuffer_;
	}

	const mat4& MeshGeometry::localTransform() const
	{
		return localTransform_;
	}

	int MeshGeometry::vertexCount() const
	{
		return vertexCount_;
	}

	int MeshGeometry::indexCount() const
	{
		return indexCount_;
	}

	BoundingBox MeshGeometry::aabb() const
	{
		return boxBV_;
	}

	BoundingSphere MeshGeometry::sphere() const
	{
		return sphereBV_;
	}

	void MeshGeometry::SetMaterial(const Material* pMat)
	{
		material_ = pMat;
	}

	void MeshGeometry::SetVertexCount(int x)
	{
		vertexCount_ = x;
	}

	void MeshGeometry::SetIndexCount(int x)
	{
		indexCount_ = x;
	}

	void MeshGeometry::SetLocalTransform(const mat4& mtx)
	{
		localTransform_ = mtx;
	}

	void MeshGeometry::SetAabb(const BoundingBox& aabb)
	{
		boxBV_ = aabb;
	}

	void MeshGeometry::SetSphere(const BoundingSphere& sph)
	{
		sphereBV_ = sph;
	}

	void MeshGeometry::SetVertexBuffer(const VertexBuffer& vb)
	{
		vertexBuffer_ = vb;
	}

	const Material* MeshGeometry::material() const
	{
		return material_;
	}
}