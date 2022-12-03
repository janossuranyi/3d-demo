#ifndef SCENE_RENDERABLE_H
#define SCENE_RENDERABLE_H

#include "engine/common.h"
#include "engine/math/bounds.h"

namespace scene {
	class Renderable
	{
	public:
		enum class Type { Mesh, Light, Camera, Particle };

		/// <summary>
		/// Default ctor
		/// </summary>
		Renderable() = default;

		/// <summary>
		/// Non copyable
		/// </summary>
		/// <param name=""></param>
		Renderable(const Renderable&) = delete;

		/// <summary>
		/// Non copyable
		/// </summary>
		/// <param name=""></param>
		Renderable& operator=(const Renderable&) = delete;

		/// <summary>
		/// default move ctor
		/// </summary>
		/// <param name=""></param>
		Renderable(Renderable&&) = default;

		/// <summary>
		/// default move assign
		/// </summary>
		/// <param name=""></param>
		/// <returns></returns>
		Renderable& operator=(Renderable&&) = default;

		virtual ~Renderable() = default;

		virtual void render(ushort pass, uint64 frame) const = 0;

		virtual math::BoundingBox aabb() const = 0;

		virtual Type getType() const = 0;
	};
}
#endif // !SCENE_RENDERABLE_H
