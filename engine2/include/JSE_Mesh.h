#ifndef JSE_MESH_H
#define JSE_MESH_H

class JseMesh {
public:
	JseMesh();
	~JseMesh();
	void AddGeometry(
		const JsVector<vec3> positions,
		const JsVector<vec2> texCoords,
		const JsVector<vec4> tangents,
		const JsVector<vec3> normals,
		const JsVector<vec4> colors,
		const JsVector<uint16_t> indices,
		const js::Material* material);

private:
	JsVector<js::MeshGeometry*> geom_;
	js::BoundingBox aabb_;
	js::BoundingSphere sphereBV_;
};
#endif // !JSE_MESH_H

