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

void JseMesh::AddGeometry(const JseVector<vec3> positions, const JseVector<vec2> texCoords, const JseVector<vec4> tangents, const JseVector<vec3> normals, const JseVector<vec4> colors, const JseVector<uint16_t> indices, const JseMaterial* material)
{
	JseMeshGeometry* geom = new JseMeshGeometry();
	geom->SetVertexCount(SCAST(int, positions.size()));
	geom->SetIndexCount(SCAST(int, indices.size()));
	
	JseGfxRenderer* hwr = appCtx.module<JseGfxRenderer>();

}
