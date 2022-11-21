#ifndef SCENE_NODE_H
#define SCENE_NODE_H

#include <vector>
#include "engine/common.h"
#include "engine/scene/renderable.h"

namespace scene {
	class Node
	{
	public:
		Node() = default;

		Node(const String& name);

		Node(const Node&) = delete;
		
		void operator=(const Node&) = delete;

		mat4 worldTransform();

		void translate(const vec3& v);
		void rotate(const quat& q);
		void scale(const vec3& s);

		quat rotation() const;
		vec3 position() const;
		vec3 scaleing() const;

		void addRenderable(Renderable* r);

		void render(ushort pass, uint64 frame);

		Node* parent();

		const Vector<Node*>& children() const;

	private:
		String					name_;
		Node*					parent_;
		Vector<Node*>			children_;
		Vector<Renderable*>		renderables_;

		mat4					m_WorldTransform_{1.0f};
		vec3					v_WorldPosition_{0.0f};
		quat					q_WorldRotation_{1.0f,0.0f,0.0f,0.0f};
		vec3					v_WorldScale_{1.0f};
		bool					needs_update_{false};
	};
}
#endif // !SCENE_NODE_H
