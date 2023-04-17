#include <glm/glm.hpp>
#include <limits>
#include "./Bounds.h"

namespace jsr {
	Bounds::Bounds() :
		origin(glm::vec3(0.0f)),
		min(glm::vec3(std::numeric_limits<float>::min())),
		max(glm::vec3(std::numeric_limits<float>::max()))
	{
	}
	Bounds::Bounds(float _min, float _max) :
		min(glm::vec3(_min)),
		max(glm::vec3(_max)),
		origin(glm::vec3(0.0f))
	{
	}
	Bounds::Bounds(float _min, float _max, const glm::vec3& _origin) : Bounds(_min,_max)
	{
		origin = _origin;
	}
	Bounds::Bounds(const glm::vec3& _min, const glm::vec3& _max, const glm::vec3& _origin) :
		min(_min),
		max(_max),
		origin(_origin)
	{
	}
	Bounds& Bounds::Extend(float k)
	{
		min.x = glm::min(min.x, k);
		min.y = glm::min(min.y, k);
		min.z = glm::min(min.z, k);

		max.x = glm::max(max.x, k);
		max.y = glm::max(max.y, k);
		max.z = glm::max(max.z, k);

		return *this;
	}
	Bounds& Bounds::Extend(const glm::vec3& v)
	{
		max = glm::max(max, v);
		min = glm::min(min, v);

		return *this;
	}
	Bounds& Bounds::Extend(const Bounds& other)
	{
		max = glm::max(max, other.max);
		min = glm::min(min, other.min);

		return *this;
	}
	int Bounds::Intersect(float fX, float fY, float fZ) const
	{
		glm::vec3 mn = min + origin;
		glm::vec3 mx = max + origin;

		if (fX < mn.x || fY < mn.y || fZ < mn.z || fX > mx.x || fY > mx.y || fZ > mx.z)
		{
			return IR_OUTSIDE;
		}
		if (fX == mn.x || fY == mn.y || fZ == mn.z || fX == mx.x || fY == mx.y || fZ == mx.z)
		{
			return IR_INTERSECT;
		}

		return IR_INSIDE;
	}
	int Bounds::Intersect(const glm::vec3& v) const
	{
		return Intersect(v.x, v.y, v.z);
	}
	float Bounds::GetRadius() const
	{
		float fmin = glm::min(glm::min(min.x, min.y), min.z);
		float fmax = glm::max(glm::max(max.x, max.y), max.z);

		return glm::abs(fmin) + glm::abs(fmax);
	}
	glm::vec3 Bounds::GetOrigin() const
	{
		return origin;
	}
	glm::vec3 Bounds::GetMin() const
	{
		return min;
	}
	glm::vec3 Bounds::GetMax() const
	{
		return max;
	}
	void Bounds::GetCorners(glm::vec3 v[8]) const
	{
		glm::vec3 mmin = min + origin;
		glm::vec3 mmax = max + origin;
		v[0] = glm::vec3(mmin.x, mmin.y, mmin.z);
		v[1] = glm::vec3(mmin.x, mmax.y, mmin.z);
		v[2] = glm::vec3(mmax.x, mmax.y, mmin.z);
		v[3] = glm::vec3(mmax.x, mmin.y, mmin.z);

		v[4] = glm::vec3(mmin.x, mmin.y, mmax.z);
		v[5] = glm::vec3(mmin.x, mmax.y, mmax.z);
		v[6] = glm::vec3(mmax.x, mmax.y, mmax.z);
		v[7] = glm::vec3(mmax.x, mmin.y, mmax.z);
	}
	glm::vec2 Bounds::IntersectRay(const glm::vec3& rOrigin, const glm::vec3& rDir)
	{
		glm::vec3 tMin = (min - rOrigin) / rDir;
		glm::vec3 tMax = (max - rOrigin) / rDir;
		glm::vec3 t1 = glm::min(tMin, tMax);
		glm::vec3 t2 = glm::max(tMin, tMax);
		float tNear = glm::max(glm::max(t1.x, t1.y), t1.z);
		float tFar = glm::min(glm::min(t2.x, t2.y), t2.z);

		return glm::vec2(tNear, tFar);
	}
	bool Bounds::operator==(const Bounds& other) const
	{
		return min == other.min && max == other.max && origin == other.origin;
	}
	bool Bounds::operator!=(const Bounds& other) const
	{
		return !(operator==(other));
	}
	Bounds Bounds::operator+(const Bounds& other) const
	{
		Bounds result{};
		result.origin = origin;
		result.min = glm::min(min, other.min);
		result.max = glm::max(max, other.max);

		return result;
	}
	Bounds& Bounds::operator+=(const Bounds& other)
	{
		Extend(other);

		return *this;
	}
}