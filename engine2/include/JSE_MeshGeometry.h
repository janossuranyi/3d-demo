#ifndef JSE_MESH_GEOMETRY_H
#define JSE_MESH_GEOMETRY_H

class JseMeshGeometry {
public:
	JseMeshGeometry();
	~JseMeshGeometry() {}
	const mat4& localTransform() const;
	const JseMaterial* material() const;
	const JseVector<vec3>& faceNormals() const;
	const JseVertexBuffer& vertexBuffer() const;
	int vertexCount() const;
	int indexCount() const;
	JseBoundingBox aabb() const;
	JseBoundingSphere sphere() const;

	void setMaterial(const JseMaterial* pMat);
	void setVertexCount(int x);
	void setIndexCount(int x);
	void setLocalTransform(const mat4& mtx);
	void setAabb(const JseBoundingBox& aabb);
	void setSphere(const JseBoundingSphere& sph);
	void setVertexBuffer(const JseVertexBuffer& vb);
private:
	int vertexCount_;
	int indexCount_;
	mat4 localTransform_;
	JseVector<vec3> faceNormals_;
	JseMaterial const* material_;
	JseBoundingSphere sphereBV_;
	JseBoundingBox boxBV_;
	JseVertexBuffer vertexBuffer_;
};
#endif // !JSE_MESH_GEOMETRY_H

