#pragma once

#include "common.h"

struct Camera
{
	enum eType { PERSPECTIVE, ORTHOGRAPHIC };
	
	eType		type;
	std::string name;
	float		aspectRatio;
	float		yfov;
	float		zfar;
	float		znear;
	float		xmag;
	float		ymag;

	glm::mat4	projectionMatrix;

	Camera() :
		type(eType::PERSPECTIVE),
		name("camera"),
		aspectRatio(16.0f / 9.0f),
		yfov(45.0f),
		znear(0.01f),
		zfar(100.0f),
		xmag(0.0f),
		ymag(0.0f) 
	{
		projectionMatrix = glm::perspective(yfov, aspectRatio, znear, zfar);
	}
};