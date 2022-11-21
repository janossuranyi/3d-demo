
#include "engine/common.h"
#include "node.h"

namespace scene {

	Node::Node(const String& name)
	{
		name_ = name;
	}

	mat4 Node::worldTransform()
	{
		if (needs_update_)
		{
			m_WorldTransform_ = glm::translate(mat4(1.0f), v_WorldPosition_);
			m_WorldTransform_ *= q_WorldRotation_;
			m_WorldTransform_ = glm::scale(m_WorldTransform_, v_WorldScale_);

			if (parent_)
			{
				m_WorldTransform_ *= parent_->worldTransform();
			}
		}

		return m_WorldTransform_;
	}

	void Node::translate(const vec3& v)
	{
		v_WorldPosition_ = v;
	}

	void Node::rotate(const quat& q)
	{
		q_WorldRotation_ = q;
	}

	void Node::scale(const vec3& s)
	{
		v_WorldScale_ = s;
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
		renderables_.push_back(r);
	}

	void Node::render(ushort pass, uint64 frame)
	{
		for (auto* e : renderables_)	e->render(pass, frame);
		for (auto* e : children_)		e->render(pass, frame);
	}

	Node* Node::parent()
	{
		return parent_;
	}

	const Vector<Node*>& Node::children() const
	{
		return children_;
	}

}
