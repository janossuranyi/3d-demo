#include <glm/glm.hpp>
#include <limits>
#include <utility>
#include "./Bounds.h"

namespace jsr {

	Sphere::Sphere(const glm::vec3& center_, float r) :
		center(center_),
		radius(r),
		radius2(r*r)
	{
	}
	bool Sphere::Contains(const glm::vec3& p) const
	{
		const auto v = c - p;
		return glm::dot(v, v) <= radius2;
	}
	float Sphere::Distance(const glm::vec3& p) const
	{
		return glm::length(p - center) - radius;
	}
	bool Sphere::RayIntersect(const glm::vec3& start, const glm::vec3& dir, float& tnear, float& tfar) const
	{
		const glm::vec3 u = glm::normalize(dir);
		const glm::vec3 oc = start - center;
		const float aoc = glm::length(oc);
		const float aoc2 = aoc * aoc;
		const float k = glm::dot(u, oc);
		float K = k * k - (aoc2 - radius2);
		
		if (K < 0)
		{
			return false;
		}

		const float d = -k;
		if (K < epsilon)
		{
			tnear = d;
			tfar = 0.0f;
			return true;
		}

		const float Ksr = glm::sqrt(K);
		tnear = (d - Ksr);
		tfar = (d + Ksr);

		return true;
	}
	bool Sphere::LineIntersect(const glm::vec3& start, const glm::vec3& end, float& tnear, float& tfar) const
	{
		const glm::vec3 v = end - start;

		if (RayIntersect(start, v, tnear, tfar))
		{
			if (tnear < 0.0f || tfar < 0.0f)
			{
				// p0 is inside the sphere
				tnear = 0.0f;
				tfar  = 0.0f;
				return true;
			}

			const float l = length(v);
			if ((l - tnear) >= 0.0f || (l - tfar) >= 0.0f)
			{
				return true;
			}
		}

		return false;
	}
	bool Sphere::Intersect(const Sphere& s) const
	{
		const glm::vec3 c = s.center - center;
		const float distance2 = glm::dot(c, c);
		const float sr = s.radius + radius;

		return distance2 <= (sr * sr);
	}
	glm::vec3 Sphere::GetCenter() const
	{
		return center;
	}
	float Sphere::GetRadius() const
	{
		return radius;
	}
	bool Sphere::Intersect(const Bounds& s) const
	{
		// get box closest point to sphere center by clamping
		const glm::vec3 v = glm::max(s.GetMin(), glm::min(center, s.GetMax()));
		const float d = dot(v, v);

		return d < radius2;
	}
	Bounds::Bounds() :
		min(glm::vec3(std::numeric_limits<float>::max())),
		max(glm::vec3(std::numeric_limits<float>::lowest()))
	{}

	Bounds::Bounds(const glm::vec3& a, const glm::vec3& b) :
		min(glm::min(a, b)),
		max(glm::max(a, b))
	{
	}
	
	Bounds& Bounds::operator<<(const glm::vec3& v)
	{
		min = glm::min(min, v);
		max = glm::max(max, v);

		return *this;
	}
	Bounds& Bounds::operator<<(const Bounds& other)
	{
		max = glm::max(max, other.max);
		min = glm::min(min, other.min);

		return *this;
	}
	bool Bounds::Contains(const glm::vec3& p) const
	{
		return glm::all(glm::greaterThanEqual(p, min)) && glm::all(glm::lessThanEqual(p, max));
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
	std::vector<glm::vec3> Bounds::GetCorners() const
	{
		using namespace glm;
		return {
			vec3(min[0], min[1], min[2]),
			vec3(min[0], min[1], max[2]),
			vec3(min[0], max[1], min[2]),
			vec3(min[0], max[1], max[2]),
			vec3(max[0], min[1], min[2]),
			vec3(max[0], min[1], max[2]),
			vec3(max[0], max[1], min[2]),
			vec3(max[0], max[1], max[2]) };
	}
	std::vector<glm::vec4> Bounds::GetHomogenousCorners() const
	{
		using namespace glm;
		return {
			vec4(min[0], min[1], min[2], 1.0f),
			vec4(min[0], min[1], max[2], 1.0f),
			vec4(min[0], max[1], min[2], 1.0f),
			vec4(min[0], max[1], max[2], 1.0f),
			vec4(max[0], min[1], min[2], 1.0f),
			vec4(max[0], min[1], max[2], 1.0f),
			vec4(max[0], max[1], min[2], 1.0f),
			vec4(max[0], max[1], max[2], 1.0f) };
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
	Bounds Bounds::Transform(const glm::mat4& trans) const
	{
		Bounds b{};
		auto corners = GetHomogenousCorners();
		b.min = glm::vec3(trans * corners[0]);
		b.max = b.min;
		for (auto i = 1; i < 8; ++i)
		{
			auto const tc = glm::vec3(trans * corners[i]);
			b << tc;
		}

		return b;
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
	bool Bounds::Empty() const
	{
		return min.x == std::numeric_limits<float>::max();
	}
}