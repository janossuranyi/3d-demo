#include "JSE.h"

void JseNode::make_dirty()
{
	dirty_ = true;
}

JseNode::JseNode() : JseNode(JseNodeType::EMPTY)
{
}

JseNode::JseNode(JseNodeType type)
{
	type_ = JseNodeType::EMPTY;
	parent_ = -1;
	parent_inst_ = nullptr;
	name_ = "noname";
}

JseNode::JseNode(JseNodeType type, const JseString& name)
{
	name_ = name;
	type_ = type;
}

JseNode::JseNode(JseNodeType type, const JseString& name, int parent)
{
	name_ = name;
	parent_ = parent;
	parent_inst_ = nullptr;
}

JseString JseNode::name() const
{
	return name_;
}

int JseNode::parent() const
{
	return parent_;
}

JseNode* JseNode::parentNode()
{
	return parent_inst_;
}

void JseNode::setParent(int x, JseNode* n)
{
	parent_ = x;
	parent_inst_ = n;
}

void JseNode::setIndex(int x)
{
	index_ = x;
}

void JseNode::setType(JseNodeType t)
{
	type_ = t;
}

const JseVector<int>& JseNode::children() const
{
	return children_;
}

void JseNode::addChild(int x)
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

void JseNode::beginAnimate()
{
	if (!animated_) {
		animated_ = true;
		saved_rotation_ = rotation_;
		saved_scale_ = scale_;
		saved_translation_ = translation_;
	}
}

void JseNode::endAnimate()
{
	if (animated_) {
		translation_ = saved_translation_;
		scale_ = saved_scale_;
		rotation_ = saved_rotation_;
		animated_ = false;
	}
}

void JseNode::scale(const vec3& v)
{
	scale_ = v;
	make_dirty();
}

void JseNode::translate(const vec3& v)
{
	translation_ = v;
	make_dirty();
}

void JseNode::rotate(const quat& v)
{
	rotation_ = v;
	make_dirty();
}

void JseNode::rotateX(float rad)
{
	rotation_ = glm::rotate(rotation_, rad, vec3(1.0f, 0.0f, 0.0f));
	make_dirty();
}

void JseNode::rotateY(float rad)
{
	rotation_ = glm::rotate(rotation_, rad, vec3(0.0f, 1.0f, 0.0f));
	make_dirty();
}

void JseNode::rotateZ(float rad)
{
	rotation_ = glm::rotate(rotation_, rad, vec3(0.0f, 0.0f, 1.0f));
	make_dirty();
}

mat4 JseNode::getTransform()
{
	if (dirty_) {
		transformMatrixCache_ = glm::translate(mat4(1.0f), translation_);
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