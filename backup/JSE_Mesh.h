#ifndef JSE_MESH_H
#define JSE_MESH_H

class JseMesh {
public:
	JseMesh();
	~JseMesh();
	void addGeometry(
		const JseVector<vec3> positions,
		const JseVector<vec2> texCoords,
		const JseVector<vec4> tangents,
		const JseVector<vec3> normals,
		const JseVector<vec4> colors,
		const JseVector<uint16_t> indices,
		const JseMaterial* material);

private:
	JseVector<JseMeshGeometry*> geom_;
	JseBoundingBox aabb_;
	JseBoundingSphere sphereBV_;
};
#endif // !JSE_MESH_H

