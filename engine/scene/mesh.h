#pragma once

#include "engine/resource/context.h"
#include "engine/common.h"
#include "engine/gfx/renderer.h"
#include "engine/math/bounds.h"

namespace scene {

	class Mesh
	{
	public:
		Mesh() = delete;
		Mesh(ctx::Context* ctx);
		Mesh(Mesh& other) = delete;

		void operator=(const Mesh& other) = delete;

		bool create(
			const vec3* positions,
			const vec2* uvs,
			const vec3* normals,
			const vec4* tangents,
			const vec4* colors,
			const ushort* indices,
			uint elements,
			gfx::PrimitiveType mode);

		const math::BoundingBox& aabb() const;
		gfx::VertexBufferHandle vb() const;
		gfx::IndexBufferHandle ib() const;
		uint baseVertex() const;
		uint startIndex() const;
		void render() const;
	private:
		math::BoundingBox aabb_{};
		gfx::VertexBufferHandle vb_{};
		gfx::IndexBufferHandle ib_{};
		uint vb_offset_{};
		uint ib_offset_{};
		uint elements_{};
		gfx::PrimitiveType mode_{};

		ctx::Context* ctx_{};
	};
}