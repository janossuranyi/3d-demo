#ifndef JSE_MESH_GEOMETRY_H
#define JSE_MESH_GEOMETRY_H

namespace js {
	class MeshGeometry {
	public:
		MeshGeometry();
		~MeshGeometry() {}
		const mat4& localTransform() const;
		const JsSharedPtr<Material> material() const;
		const JsVector<vec3>& faceNormals() const;
		int vertexCount() const;
		int indexCount() const;
		BoundingBox aabb() const;
		BoundingSphere sphere() const;

		void SetMaterial(JsSharedPtr<Material>& pMat);
		void SetVertexCount(int x);
		void SetIndexCount(int x);
		void SetLocalTransform(const mat4& mtx);
		void SetAabb(const BoundingBox& aabb);
		void SetSphere(const BoundingSphere& sph);
	private:
		int vertexCount_;
		int indexCount_;
		mat4 localTransform_;
		JsSharedPtr<Material> material_;
		BoundingSphere sphereBV_;
		BoundingBox boxBV_;
		CacheHandle vertexCache_;
		CacheHandle indexCache_;
		JsVector<GfxDrawVert> vertices_;
		JsVector<int16_t> indices_;
		JsVector<vec3> faceNormals_;
	};
}
#endif // !JSE_MESH_GEOMETRY_H

