
#include "engine/common.h"
#include "node.h"

namespace scene {

	Node::Node(const String& name, gfx::Renderer* hwr)
	{
		name_ = name;
		hwr_ = hwr;
	}

	mat4 Node::worldTransform()
	{
		if (need_to_update_)
		{
			m_WorldTransform_ = glm::translate(mat4(1.0f), v_WorldPosition_);
			m_WorldTransform_ *= mat4(q_WorldRotation_);
			m_WorldTransform_ = glm::scale(m_WorldTransform_, v_WorldScale_);

			if (parent_)
			{
				m_WorldTransform_ *= parent_->worldTransform();
			}

			for (auto* e : children_) e->needToUpdate();

			need_to_update_ = false;
		}

		return m_WorldTransform_;
	}

	const mat4& Node::worldTransformRef()
	{
		if (need_to_update_) worldTransform();

		return m_WorldTransform_;
	}

	void Node::translate(const vec3& v)
	{
		v_WorldPosition_ = v;
		need_to_update_ = true;
	}

	void Node::rotate(const quat& q)
	{
		q_WorldRotation_ = q;
		need_to_update_ = true;
	}

	void Node::scale(const vec3& s)
	{
		v_WorldScale_ = s;
		need_to_update_ = true;
	}

	quat Node::rotation() const
	{
		return q_WorldRotation_;
	}

	vec3 Node::position() const
	{
		return v_WorldPosition_;
	}

	vec3 Node::scaleing() const
	{
		return v_WorldScale_;
	}

	void Node::addRenderable(Renderable* r)
	{		
		for (auto p : renderables_)
		{
			if (p == r) return;
		}

		renderables_.push_back(r);
		aabb_.merge(r->aabb());
	}

	void Node::addChild(Node* node)
	{
		if (node->parent()) return;

		node->parent_ = this;
		children_.push_back(node);
	}

	Node* Node::parent()
	{
		return parent_;
	}

	const Vector<Node*>& Node::children() const
	{
		return children_;
	}

	const Vector<Renderable*>& Node::renderables() const
	{
		return renderables_;
	}

	math::BoundingBox Node::aabb() const
	{
		return aabb_;
	}

	void Node::needToUpdate()
	{
		need_to_update_ = true;
	}

}
