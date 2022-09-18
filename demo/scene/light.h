#pragma once

#include "common.h"
#include <cmath>

struct xLight
{
	enum class eType { DIRECTIONAL, POINT, SPOT };

	eType			type;
	glm::vec3		color;
	float			intensity;
	float			range;				// attenuation = max( min( 1.0 - ( current_distance / range )4, 1 ), 0 ) / current_distance2
	float			innerConeAngle;
	float			cosInnerConeAngle;
	float			outerConeAngle;
	float			cosOuterConeAngle;

	void			setInnerConeAngle(float x);
	void			setOuterConeAngle(float x);

	xLight() :
		type(eType::POINT),
		color(1.0f),
		intensity(1.0f),
		range(0.0f),
		innerConeAngle(0.0f),
		outerConeAngle(M_PI / 4.0f),
		cosInnerConeAngle(std::cos(innerConeAngle)),
		cosOuterConeAngle(std::cos(cosOuterConeAngle)) {}
};

/*
// These two values can be calculated on the CPU and passed into the shader
float lightAngleScale = 1.0f / max(0.001f, cos(innerConeAngle) - cos(outerConeAngle));
float lightAngleOffset = -cos(outerConeAngle) * lightAngleScale;

// Then, in the shader:
float cd = dot(spotlightDir, normalizedLightVector);
float angularAttenuation = saturate(cd * lightAngleScale + lightAngleOffset);
angularAttenuation *= angularAttenuation;
*/