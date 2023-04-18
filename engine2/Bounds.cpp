#include <glm/glm.hpp>
#include <limits>
#include <utility>
#include "./Bounds.h"

namespace jsr {

	Bounds::Bounds() :
		min(glm::vec3(std::numeric_limits<float>::max())),
		max(-glm::vec3(std::numeric_limits<float>::max()))
	{
	}
	Bounds::Bounds(float _min, float _max) :
		min(glm::vec3(_min)),
		max(glm::vec3(_max))
	{
	}
	Bounds::Bounds(const glm::vec3& _min, const glm::vec3& _max) :
		min(_min),
		max(_max)
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
		glm::vec3 mn = min;
		glm::vec3 mx = max;

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
		return glm::length(max - min) * 0.5f;
	}
	glm::vec3 Bounds::GetCenter() const
	{
		return (min + max) * 0.5f;
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
		glm::vec3 mmin = min;
		glm::vec3 mmax = max;
		v[0] = glm::vec3(mmin.x, mmin.y, mmin.z);
		v[1] = glm::vec3(mmin.x, mmax.y, mmin.z);
		v[2] = glm::vec3(mmax.x, mmax.y, mmin.z);
		v[3] = glm::vec3(mmax.x, mmin.y, mmin.z);

		v[4] = glm::vec3(mmin.x, mmin.y, mmax.z);
		v[5] = glm::vec3(mmin.x, mmax.y, mmax.z);
		v[6] = glm::vec3(mmax.x, mmax.y, mmax.z);
		v[7] = glm::vec3(mmax.x, mmin.y, mmax.z);
	}
	bool Bounds::RayIntersect(const glm::vec3& start, const glm::vec3& rayDir, float& tnear, float& tfar)
	{
		glm::vec3 invDir = 1.0f / rayDir;
		glm::vec3 tMin = (min - start) * invDir;
		glm::vec3 tMax = (max - start) * invDir;
		glm::vec3 t1 = glm::min(tMin, tMax);
		glm::vec3 t2 = glm::max(tMin, tMax);
		tnear = glm::max(glm::max(t1.x, t1.y), t1.z);
		tfar = glm::min(glm::min(t2.x, t2.y), t2.z);

		return tfar > tnear;
	}
	/*
	bool Bounds::RayIntersect(const glm::vec3& start, const glm::vec3& dir, float& tmin, float& tmax)
	{
		tmax = std::numeric_limits<float>::max();
		tmin = -tmax;
		for (int a = 0; a < 3; a++)
		{
			float invD = 1.0f / dir[a];
			float t0 = (min[a] - start[a]) * invD;
			float t1 = (max[a] - start[a]) * invD;
			if (invD < 0)
			{
				std::swap(t0, t1);
			}
			tmin = t0 > tmin ? t0 : tmin;
			tmax = t1 < tmax ? t1 : tmax;
			if (tmax <= tmin)
			{
				return false;
			}
		}
		return true;
	}
	*/
	bool Bounds::operator==(const Bounds& other) const
	{
		return min == other.min && max == other.max;
	}
	bool Bounds::operator!=(const Bounds& other) const
	{
		return !(operator==(other));
	}
	Bounds Bounds::operator+(const Bounds& other) const
	{
		Bounds result{};
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