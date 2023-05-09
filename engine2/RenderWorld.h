#ifndef JSR_RENDERWORLD_H
#define JSR_RENDERWORLD_H

#include <vector>
#include <tiny_gltf.h>
#include "./Model.h"
#include "./Node3D.h"

namespace jsr {
	class RenderWorld
	{
	public:
		RenderWorld() = default;
		~RenderWorld() {};

		bool LoadFromGLTF(const std::string& filename);
		void RenderView(viewDef_t* view);
	private:
		Bounds worldBounds;
		std::vector<Node3D*> nodes;
	};
}
#endif // !JSR_RENDERWORLD_H
