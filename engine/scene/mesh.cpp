#include "engine/scene/mesh.h"
#include "engine/gfx/vertex_cache.h"

namespace scene {

	Mesh::Mesh(ctx::Context* ctx) : ctx_(ctx)
	{
		assert(ctx_ != nullptr);
	}

	bool Mesh::create(const vec3* positions, const vec2* uvs, const vec3* normals, const vec4* tangents, const vec4* colors, const ushort* indices, uint elements, gfx::PrimitiveType mode)
	{
		gfx::Renderer* hwr = ctx_->hwr();
		gfx::VertexCache* vc = ctx_->vertexCache();

		return false;
	}

	const math::BoundingBox& Mesh::aabb() const
	{
		return aabb_;
	}

	gfx::VertexBufferHandle Mesh::vb() const
	{
		return vb_;
	}

	gfx::IndexBufferHandle Mesh::ib() const
	{
		return ib_;
	}

	uint Mesh::baseVertex() const
	{
		return vb_offset_;
	}

	uint Mesh::startIndex() const
	{
		return ib_offset_;
	}

	void Mesh::render() const
	{
	}

}