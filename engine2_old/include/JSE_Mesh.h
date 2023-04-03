#ifndef JSE_MESH_H
#define JSE_MESH_H

namespace js {

	class Mesh {
	public:
		Mesh();
		~Mesh();
	private:
		JsVector<MeshGeometry*> geom_;
		BoundingBox aabb_;
		BoundingSphere sphereBV_;
	};

}
#endif // !JSE_MESH_H

