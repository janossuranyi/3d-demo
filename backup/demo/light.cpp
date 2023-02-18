#include "scene/light.h"

void xLight::setInnerConeAngle(float x)
{
	innerConeAngle = x;
	cosInnerConeAngle = glm::cos(x);
}

void xLight::setOuterConeAngle(float x)
{
	outerConeAngle = x;
	cosOuterConeAngle = glm::cos(x);
}
