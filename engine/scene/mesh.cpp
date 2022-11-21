#include "engine/scene/mesh.h"
#include "engine/gfx/vertex_cache.h"
#include "engine/gfx/memory.h"

namespace scene {

	Mesh::Mesh(ctx::Context* ctx, bool astatic) : ctx_(ctx),static_(astatic)
	{
		assert(ctx_ != nullptr);
	}

	bool Mesh::create(const vec3* positions, const vec2* uvs, const vec3* normals, const vec4* tangents, const vec4* colors, const ushort* indices, uint numVerts, uint elements, gfx::PrimitiveType mode)
	{
		gfx::Renderer* hwr = ctx_->hwr();
		gfx::VertexCache* vc = ctx_->vertexCache();

		verts_.reset(new gfx::DrawVert[numVerts]);
		for (uint i = 0; i < numVerts; ++i)
		{
			if (positions)		verts_[i].setPosition(positions[i]);
			if (uvs)			verts_[i].setTexCoord(uvs[i]);
			if (normals)		verts_[i].setNormal(normals[i]);
			if (tangents)		verts_[i].setTangent(tangents[i]);
			if (colors)			verts_[i].setColor(colors[i]);
		}

		if (static_) {
			vtxc_ = vc->allocStaticVertex(gfx::Memory(verts_.get(), numVerts * sizeof(gfx::DrawVert)));
			verts_.reset(nullptr);
		}

		if (elements && static_) {
			idxc_ = vc->allocStaticIndex(gfx::Memory(indices, elements * sizeof(ushort)));
		}

		if (elements && !static_)
		{
			idxs_.reset(new ushort[elements]);
			std::memcpy(&idxs_[0], indices, elements * sizeof(ushort));
		}

		elements_ = elements;
		vertices_ = numVerts;
		mode_ = mode;

		return true;
	}

	const math::BoundingBox& Mesh::aabb() const
	{
		return aabb_;
	}

	void Mesh::render(ushort pass) const
	{
		if (!material_ || !vtxc_) return;

		gfx::Renderer& hwr = *ctx_->hwr();
		gfx::VertexCache& vc = *ctx_->vertexCache();

		uint vertex_count = 0;
		uint base_vertex{}, vb_size{};
		uint ib_offset{}, idx_count{};
		gfx::VertexBufferHandle vb = vc.getVertexBuffer<gfx::DrawVert>(vtxc_, base_vertex, vb_size);

		hwr.setVertexBuffer(vb);

		if (elements_)
		{
			gfx::IndexBufferHandle ib = vc.getIndexBuffer<ushort>(idxc_, ib_offset, idx_count);
			hwr.setIndexBuffer(ib);
			vertex_count = elements_;
		}
		else
		{
			vertex_count = vertices_;
		}

		for (uint i = 0; i < material_->textures.size(); ++i)
		{
			hwr.setTexture(material_->textures[i].handle, i);
		}
		hwr.setPrimitiveType(mode_);
		hwr.setVertexDecl(hwr.defaultVertexDecl());
		hwr.submit(pass, material_->program, vertex_count, base_vertex, ib_offset);
	}

	void Mesh::setMaterial(const gfx::Material* material)
	{
		material_ = material;
	}

	const gfx::Material* Mesh::material() const
	{
		return material_;
	}
}