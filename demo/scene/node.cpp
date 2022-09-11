#include "node.h"

void Node::applyMatrix(const glm::mat4& matrixData)
{
	glm::vec3 _scale, _translation, _skew;
	glm::vec4 _persp;
	glm::quat _rotation;

	if (glm::decompose(matrixData, _scale, _rotation, _translation, _skew, _persp))
	{
		this->scale = _scale;
		this->rotation = glm::normalize(_rotation);
		this->translation = _translation;
	}
	changed = true;
}

void Node::applyTranslationAnimation(const glm::vec3& translation)
{
	animationTranslation = translation;
	changed = true;
}

void Node::applyRotationAnimation(const glm::quat& rotation)
{
	animationRotation = rotation;
	changed = true;
}

void Node::applyScaleAnimation(const glm::vec3& scale)
{
	animationScale = scale;
	changed = true;
}

void Node::resetTransform()
{
	rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
	scale = glm::vec3(1.f, 1.f, 1.f);
	translation = glm::vec3(0.f, 0.f, 0.f);
	changed = true;
}

glm::mat4 Node::getLocalTransform()
{
	if (changed)
	{
		if (hasMatrix && !animated)
		{
			transform = matrix;
		}
		else
		{
			transform = glm::mat4(1.0f);
			const glm::vec3 _translation = animated ? animationTranslation : translation;
			const glm::vec3 _scale = animated ? animationScale : scale;
			const glm::quat _rotation = animated ? animationRotation : rotation;
			const glm::mat4 _rotm = glm::mat4(_rotation);

			transform = glm::translate(transform, _translation);
			transform = transform * _rotm;
			transform = glm::scale(transform, _scale);
		}
		changed = false;
	}

	return transform;
}


