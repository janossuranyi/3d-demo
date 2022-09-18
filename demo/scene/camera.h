#pragma once

#include "common.h"

struct xCamera
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

	xCamera() :
		type(eType::PERSPECTIVE),
		name("camera"),
		aspectRatio(16.0f / 9.0f),
		yfov(45.0f),
		znear(0.01f),
		zfar(100.0f),
		xmag(0.0f),
		ymag(0.0f) 
	{
		update();
	}

	inline void update()
	{
		if (type == eType::PERSPECTIVE)
		{
			if (zfar == 0.0f) zfar = 100.0f;

			const float tanHalfYfov = glm::tan(0.5f * yfov);

			glm::mat4 m(1.0f);
			m[0][0] = 1.0f / (aspectRatio * tanHalfYfov);
			m[1][1] = 1.0f / tanHalfYfov;
			m[2][2] = (zfar + znear) / (znear - zfar);
			m[3][2] = (2.0f * zfar * znear) / (znear - zfar);

			projectionMatrix = m;
		}
		else
		{
			glm::mat4 m(1.0f);
			m[0][0] = 1.0f / xmag;
			m[1][1] = 1.0f / ymag;
			m[2][2] = 2.0f / (znear - zfar);
			m[3][2] = (zfar + znear) / (znear - zfar);

			projectionMatrix = m;
		}
	}
};