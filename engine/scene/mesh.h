#pragma once

#include <memory>
#include "engine/resource/context.h"
#include "engine/common.h"
#include "engine/gfx/draw_vert.h"
#include "engine/gfx/renderer.h"
#include "engine/gfx/material.h"
#include "engine/math/bounds.h"
#include "engine/scene/renderable.h"

namespace scene {

	class Mesh : public Renderable
	{
	public:
		Mesh() = delete;
		Mesh(ctx::Context* ctx, bool astatic = true);
		
		const math::BoundingBox& aabb() const;

		void					setMaterial(const gfx::Material* material);

		const gfx::Material*	material() const;

		bool create(
			const vec3* positions,
			const vec2* texCoords,
			const vec3* normals,
			const vec4* tangents,
			const vec4* colors,
			const ushort* indices,
			uint numVerts,
			uint elements,
			gfx::PrimitiveType mode);


		void					render(ushort pass, uint64 frame) const override;

	private:
		bool								static_{};
		math::BoundingBox					aabb_{};
		gfx::PrimitiveType					mode_{};
		gfx::vtxCacheHandle					vtxc_{};
		gfx::vtxCacheHandle					idxc_{};
		uint								elements_;
		uint								vertices_;
		ctx::Context*						ctx_{};
		gfx::Material const*				material_{};
		std::unique_ptr<gfx::DrawVert[]>	verts_{};
		std::unique_ptr<ushort[]>			idxs_{};
	};
}