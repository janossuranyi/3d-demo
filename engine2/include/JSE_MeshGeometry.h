#ifndef JSE_MESH_GEOMETRY_H
#define JSE_MESH_GEOMETRY_H

namespace js {
	class MeshGeometry {
	public:
		MeshGeometry();
		~MeshGeometry() {}
		const mat4& localTransform() const;
		const Material* material() const;
		const JsVector<vec3>& faceNormals() const;
		const VertexBuffer& vertexBuffer() const;
		int vertexCount() const;
		int indexCount() const;
		BoundingBox aabb() const;
		BoundingSphere sphere() const;

		void SetMaterial(const Material* pMat);
		void SetVertexCount(int x);
		void SetIndexCount(int x);
		void SetLocalTransform(const mat4& mtx);
		void SetAabb(const BoundingBox& aabb);
		void SetSphere(const BoundingSphere& sph);
		void SetVertexBuffer(const VertexBuffer& vb);
	private:
		int vertexCount_;
		int indexCount_;
		mat4 localTransform_;
		JsVector<vec3> faceNormals_;
		Material const* material_;
		BoundingSphere sphereBV_;
		BoundingBox boxBV_;
		VertexBuffer vertexBuffer_;
	};
}
#endif // !JSE_MESH_GEOMETRY_H

