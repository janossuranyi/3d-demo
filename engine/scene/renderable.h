#ifndef SCENE_RENDERABLE_H
#define SCENE_RENDERABLE_H

#include "engine/common.h"
#include "engine/math/bounds.h"

namespace scene {
	class Renderable
	{
	public:
		enum class Type { Mesh, Light, Camera };

		Renderable() = default;
		Renderable(const Renderable&) = delete;
		void operator=(const Renderable&) = delete;
		virtual ~Renderable() = default;
		virtual void render(ushort pass, uint64 frame) const = 0;
		virtual math::BoundingBox aabb() const = 0;
		virtual Type getType() const = 0;
	};
}
#endif // !SCENE_RENDERABLE_H
