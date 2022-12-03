#ifndef SCENE_NODE_H
#define SCENE_NODE_H

#include <vector>
#include <functional>
#include "engine/common.h"
#include "engine/scene/renderable.h"
#include "engine/gfx/renderer.h"
#include "engine/math/bounds.h"

namespace scene {
	class Node
	{
	public:
		Node() = delete;

		Node(const String& name, gfx::Renderer* hwr);

		Node(const Node&) = delete;
		
		Node& operator=(const Node&) = delete;

		mat4 worldTransform();
		const mat4& worldTransformRef();

		void translate(const vec3& v);
		void rotate(const quat& q);
		void scale(const vec3& s);

		quat rotation() const;
		vec3 position() const;
		vec3 scaleing() const;

		void addRenderable(Renderable* r);

		void addChild(Node* node);

		Node* parent();

		const Vector<Node*>&		children() const;
		const Vector<Renderable*>&	renderables() const;
		math::BoundingBox			aabb() const;
		void needToUpdate();

	private:
		String					name_;
		Node*					parent_{};
		Vector<Node*>			children_;
		Vector<Renderable*>		renderables_;

		mat4					m_WorldTransform_{1.0f};
		vec3					v_WorldPosition_{0.0f};
		quat					q_WorldRotation_{1.0f,0.0f,0.0f,0.0f};
		vec3					v_WorldScale_{1.0f};
		bool					need_to_update_{false};
		gfx::Renderer*			hwr_{};
		math::BoundingBox		aabb_{};
	};
}
#endif // !SCENE_NODE_H
