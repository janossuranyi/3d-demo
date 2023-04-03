#include "JSE.h"
#include <glm/gtx/matrix_decompose.hpp>

namespace js {

	void Node::make_dirty()
	{
		dirty_ = true;
	}

	Node::Node() : pParent_(), name_()
	{
	}


	Node::Node(Type type, const JsString& name, Node* pParent)
	{
		type_ = type;
		name_ = name;
		pParent_ = pParent;
	}

	JsString Node::name() const
	{
		return name_;
	}

	Node* Node::parentNode()
	{
		return pParent_;
	}

	void Node::SetMatrix(const mat4& m)
	{
		vec3 scale{}, translation{}, skew{};
		vec4 persp{};
		quat orient{};

		if (glm::decompose(m, scale, orient, translation, skew, persp))
		{
			scale_ = scale;
			translation_ = translation;
			rotation_ = orient;
		}
	}

	void Node::SetParent(Node* n)
	{
		pParent_ = n;
	}

	void Node::SetIndex(int x)
	{
		index_ = x;
	}

	void Node::SetType(Type t)
	{
		type_ = t;
	}

	const JsVector<int>& Node::children() const
	{
		return children_;
	}

	void Node::AddChild(int x)
	{
		children_.push_back(x);
	}

	vec3 Node::translation() const
	{
		return translation_;
	}

	vec3 Node::scale() const
	{
		return scale_;
	}

	quat Node::rotation() const
	{
		return rotation_;
	}

	bool Node::animated() const
	{
		return animated_;
	}

	void Node::BeginAnimate()
	{
		if (!animated_) {
			animated_ = true;
			saved_rotation_ = rotation_;
			saved_scale_ = scale_;
			saved_translation_ = translation_;
		}
	}

	void Node::EndAnimate()
	{
		if (animated_) {
			translation_ = saved_translation_;
			scale_ = saved_scale_;
			rotation_ = saved_rotation_;
			animated_ = false;
		}
	}

	void Node::Scale(const vec3& v)
	{
		scale_ = v;
		make_dirty();
	}

	void Node::Translate(const vec3& v)
	{
		translation_ = v;
		make_dirty();
	}

	void Node::Rotate(const quat& v)
	{
		rotation_ = v;
		make_dirty();
	}

	void Node::RotateX(float rad)
	{
		rotation_ = glm::rotate(rotation_, rad, vec3(1.0f, 0.0f, 0.0f));
		make_dirty();
	}

	void Node::RotateY(float rad)
	{
		rotation_ = glm::rotate(rotation_, rad, vec3(0.0f, 1.0f, 0.0f));
		make_dirty();
	}

	void Node::RotateZ(float rad)
	{
		rotation_ = glm::rotate(rotation_, rad, vec3(0.0f, 0.0f, 1.0f));
		make_dirty();
	}

	mat4 Node::GetTransform()
	{
		if (dirty_) {
			transformMatrixCache_ = pParent_ ? pParent_->GetTransform() : mat4(1.0f);
			transformMatrixCache_ = glm::translate(transformMatrixCache_, translation_);
			transformMatrixCache_ *= mat4(rotation_);
			transformMatrixCache_ = glm::scale(transformMatrixCache_, scale_);
			dirty_ = false;
		}
		return transformMatrixCache_;
	}

	int Node::index() const
	{
		return index_;
	}

	Node::Type Node::type() const
	{
		return type_;
	}

	bool Node::isCamera() const
	{
		return type_ == Type::CAMERA;
	}

	bool Node::isLight() const
	{
		return type_ == Type::LIGHT;
	}

	bool Node::isMesh() const
	{
		return type_ == Type::MESH;
	}

	bool Node::empty() const
	{
		if (isCamera()) return false;
		else if (isLight()) return false;
		else if (isMesh()) return false;

		return true;
	}

	int Node::operator[](size_t x) const
	{
		return children_.at(x);
	}

}