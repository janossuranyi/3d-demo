#include "JSE.h"

JseMesh::JseMesh()
{
}

JseMesh::~JseMesh()
{
	auto it = geom_.begin();
	for (; it != geom_.end(); it++) {
		delete* it;
	}
}

void JseMesh::AddGeometry(const JsVector<vec3> positions, const JsVector<vec2> texCoords, const JsVector<vec4> tangents, const JsVector<vec3> normals, const JsVector<vec4> colors, const JsVector<uint16_t> indices, const JseMaterial* material)
{
	js::MeshGeometry* geom = new js::MeshGeometry();
	geom->SetVertexCount(SCAST(int, positions.size()));
	geom->SetIndexCount(SCAST(int, indices.size()));
	
	js::GfxRenderer* hwr = appCtx.module<js::GfxRenderer>();

}
