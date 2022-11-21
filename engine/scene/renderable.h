#ifndef SCENE_RENDERABLE_H
#define SCENE_RENDERABLE_H

#include "engine/common.h"

namespace scene {
	class Renderable
	{
	public:
		Renderable(const Renderable&) = delete;
		void operator=(const Renderable&) = delete;
		virtual ~Renderable() = default;
		virtual void render(ushort pass, uint64 frame) const = 0;
	};
}
#endif // !SCENE_RENDERABLE_H
