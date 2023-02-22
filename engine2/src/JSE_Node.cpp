#include "JSE.h"
#include <glm/gtx/matrix_decompose.hpp>

void JseNode::make_dirty()
{
	dirty_ = true;
}

JseNode::JseNode() : pParent_(), name_()
{
}


JseNode::JseNode(JseNodeType type, const JseString& name, JseNode* pParent)
{
	type_ = type;
	name_ = name;
	pParent_ = pParent;
}

JseString JseNode::name() const
{
	return name_;
}

JseNode* JseNode::parentNode()
{
	return pParent_;
}

void JseNode::SetMatrix(const mat4& m)
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

void JseNode::SetParent(JseNode* n)
{
	pParent_ = n;
}

void JseNode::SetIndex(int x)
{
	index_ = x;
}

void JseNode::SetType(JseNodeType t)
{
	type_ = t;
}

const JseVector<int>& JseNode::children() const
{
	return children_;
}

void JseNode::AddChild(int x)
{
	children_.push_back(x);
}

vec3 JseNode::translation() const
{
	return translation_;
}

vec3 JseNode::scale() const
{
	return scale_;
}

quat JseNode::rotation() const
{
	return rotation_;
}

bool JseNode::animated() const
{
	return animated_;
}

void JseNode::BeginAnimate()
{
	if (!animated_) {
		animated_ = true;
		saved_rotation_ = rotation_;
		saved_scale_ = scale_;
		saved_translation_ = translation_;
	}
}

void JseNode::EndAnimate()
{
	if (animated_) {
		translation_ = saved_translation_;
		scale_ = saved_scale_;
		rotation_ = saved_rotation_;
		animated_ = false;
	}
}

void JseNode::Scale(const vec3& v)
{
	scale_ = v;
	make_dirty();
}

void JseNode::Translate(const vec3& v)
{
	translation_ = v;
	make_dirty();
}

void JseNode::Rotate(const quat& v)
{
	rotation_ = v;
	make_dirty();
}

void JseNode::RotateX(float rad)
{
	rotation_ = glm::rotate(rotation_, rad, vec3(1.0f, 0.0f, 0.0f));
	make_dirty();
}

void JseNode::RotateY(float rad)
{
	rotation_ = glm::rotate(rotation_, rad, vec3(0.0f, 1.0f, 0.0f));
	make_dirty();
}

void JseNode::RotateZ(float rad)
{
	rotation_ = glm::rotate(rotation_, rad, vec3(0.0f, 0.0f, 1.0f));
	make_dirty();
}

mat4 JseNode::GetTransform()
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

int JseNode::index() const
{
	return index_;
}

JseNodeType JseNode::type() const
{
	return type_;
}

bool JseNode::isCamera() const
{
	return type_ == JseNodeType::CAMERA;
}

bool JseNode::isLight() const
{
	return type_ == JseNodeType::LIGHT;
}

bool JseNode::isMesh() const
{
	return type_ == JseNodeType::MESH;
}

bool JseNode::empty() const
{
	if (isCamera()) return false;
	else if (isLight()) return false;
	else if (isMesh()) return false;

	return true;
}

int JseNode::operator[](size_t x) const
{
	return children_.at(x);
}