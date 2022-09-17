#pragma once

#include "common.h"

struct xNode
{
	enum class eType :uint { Mesh, Light, Camera, Transform };

	eType				type;
	int					value;
	std::string			name;
	std::vector<uint>	children;

	// gltf
	glm::vec3			translation;
	glm::quat			rotation;
	glm::vec3			scale;
	glm::mat4			matrix;
	bool				hasMatrix;

	glm::mat4			worldTransform;
	glm::mat4			inverseWorldTransform;
	glm::mat4			normalMatrix;
	glm::mat4			transform;

	bool				changed;
	bool				animated;
	glm::quat			animationRotation;
	glm::vec3			animationTranslation;
	glm::vec3			animationScale;

	void		applyMatrix(const glm::mat4& matrixData);

	void		applyRotationAnimation(const glm::quat& rotation);

	void		applyScaleAnimation(const glm::vec3& scale);

	void		applyTranslationAnimation(const glm::vec3& translation);

	void		resetTransform();

	glm::mat4	getLocalTransform();

	xNode(xNode&) = delete;

	xNode(xNode&&) = default;

	xNode& operator=(xNode&) = delete;

	xNode& operator=(xNode&&) = default;

	xNode() :
		type(eType::Transform),
		value(-1),
		changed(true),
		animated(false),
		hasMatrix(false),
		matrix(1.0f),
		transform(1.0f),
		translation(0.f,0.f,0.f),
		rotation(1.f,0.f,0.f,0.f),
		scale(1.f,1.f,1.f),
		worldTransform(1.0f),
		normalMatrix(1.0f),
		animationRotation(1.f,0.f,0.f,0.f),
		animationTranslation(0.f,0.f,0.f),
		animationScale(1.f,1.f,1.f),
		name("<empty>")
	{	
		inverseWorldTransform = glm::inverse(worldTransform);
	}

	xNode(const std::string& _name) : xNode() {
		name = _name;
	}

};