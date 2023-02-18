#include "JSE.h"

JseMeshGeometry::JseMeshGeometry() :
	vertexCount_(0),
	indexCount_(0),
	material_(),
	sphereBV_(),
	boxBV_(),
	localTransform_(1.f),
	vertexBuffer_()
{
}

const JseVector<vec3>& JseMeshGeometry::faceNormals() const
{
	return faceNormals_;
}

const JseVertexBuffer& JseMeshGeometry::vertexBuffer() const
{
	return vertexBuffer_;
}

const mat4& JseMeshGeometry::localTransform() const
{
	return localTransform_;
}

int JseMeshGeometry::vertexCount() const
{
	return vertexCount_;
}

int JseMeshGeometry::indexCount() const
{
	return indexCount_;
}

JseBoundingBox JseMeshGeometry::aabb() const
{
	return boxBV_;
}

JseBoundingSphere JseMeshGeometry::sphere() const
{
	return sphereBV_;
}

void JseMeshGeometry::setMaterial(const JseMaterial* pMat)
{
	material_ = pMat;
}

void JseMeshGeometry::setVertexCount(int x)
{
	vertexCount_ = x;
}

void JseMeshGeometry::setIndexCount(int x)
{
	indexCount_ = x;
}

void JseMeshGeometry::setLocalTransform(const mat4& mtx)
{
	localTransform_ = mtx;
}

void JseMeshGeometry::setAabb(const JseBoundingBox& aabb)
{
	boxBV_ = aabb;
}

void JseMeshGeometry::setSphere(const JseBoundingSphere& sph)
{
	sphereBV_ = sph;
}

void JseMeshGeometry::setVertexBuffer(const JseVertexBuffer& vb)
{
	vertexBuffer_ = vb;
}

const JseMaterial* JseMeshGeometry::material() const
{
	return material_;
}
