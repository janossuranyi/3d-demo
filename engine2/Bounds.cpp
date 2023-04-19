#include <glm/glm.hpp>
#include <limits>
#include "./Bounds.h"

namespace jsr {

	Sphere::Sphere(const glm::vec3& center_, float r) :
		_center(center_),
		_radius(r),
		_radius2(r*r)
	{
	}
	bool Sphere::Contains(const glm::vec3& p) const
	{
		const auto v = _center - p;
		return glm::dot(v, v) <= _radius2;
	}
	float Sphere::Distance(const glm::vec3& p) const
	{
		return glm::length(p - _center) - _radius;
	}
	bool Sphere::RayIntersect(const glm::vec3& start, const glm::vec3& dir, float& tnear, float& tfar) const
	{
		const glm::vec3 u = glm::normalize(dir);
		const glm::vec3 oc = start - _center;
		const float aoc = glm::length(oc);
		const float aoc2 = aoc * aoc;
		const float k = glm::dot(u, oc);
		float K = k * k - (aoc2 - _radius2);
		
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
		const glm::vec3 c = s._center - _center;
		const float distance2 = glm::dot(c, c);
		const float sr = s._radius + _radius;

		return distance2 <= (sr * sr);
	}
	glm::vec3 Sphere::GetCenter() const
	{
		return _center;
	}
	float Sphere::GetRadius() const
	{
		return _radius;
	}
	bool Sphere::Intersect(const Bounds& s) const
	{
		// get box closest point to sphere center by clamping
		const glm::vec3 v = glm::max(s.GetMin(), glm::min(_center, s.GetMax()));
		const float d = dot(v, v);

		return d < _radius2;
	}
	Bounds::Bounds() :
		_min(glm::vec3(std::numeric_limits<float>::max())),
		_max(glm::vec3(std::numeric_limits<float>::lowest()))
	{}

	Bounds::Bounds(const glm::vec3& a, const glm::vec3& b) :
		_min(glm::min(a, b)),
		_max(glm::max(a, b))
	{
	}
	
	Bounds& Bounds::operator<<(const glm::vec3& v)
	{
		_min = glm::min(_min, v);
		_max = glm::max(_max, v);

		return *this;
	}
	Bounds& Bounds::operator<<(const Bounds& other)
	{
		_max = glm::max(_max, other._max);
		_min = glm::min(_min, other._min);

		return *this;
	}
	bool Bounds::Contains(const glm::vec3& p) const
	{
		return glm::all(glm::greaterThanEqual(p, _min)) && glm::all(glm::lessThanEqual(p, _max));
	}
	float Bounds::GetRadius() const
	{
		return glm::length(_max - _min) * 0.5f;
	}
	glm::vec3 Bounds::GetCenter() const
	{
		return (_min + _max) * 0.5f;
	}
	glm::vec3 Bounds::GetMin() const
	{
		return _min;
	}
	glm::vec3 Bounds::GetMax() const
	{
		return _max;
	}
	std::vector<glm::vec3> Bounds::GetCorners() const
	{
		using namespace glm;
		return {
			vec3(_min[0], _min[1], _min[2]),
			vec3(_min[0], _min[1], _max[2]),
			vec3(_min[0], _max[1], _min[2]),
			vec3(_min[0], _max[1], _max[2]),
			vec3(_max[0], _min[1], _min[2]),
			vec3(_max[0], _min[1], _max[2]),
			vec3(_max[0], _max[1], _min[2]),
			vec3(_max[0], _max[1], _max[2]) };
	}
	std::vector<glm::vec4> Bounds::GetHomogenousCorners() const
	{
		using namespace glm;
		return {
			vec4(_min[0], _min[1], _min[2], 1.0f),
			vec4(_min[0], _min[1], _max[2], 1.0f),
			vec4(_min[0], _max[1], _min[2], 1.0f),
			vec4(_min[0], _max[1], _max[2], 1.0f),
			vec4(_max[0], _min[1], _min[2], 1.0f),
			vec4(_max[0], _min[1], _max[2], 1.0f),
			vec4(_max[0], _max[1], _min[2], 1.0f),
			vec4(_max[0], _max[1], _max[2], 1.0f) };
	}
	bool Bounds::RayIntersect(const glm::vec3& start, const glm::vec3& invDir, float& tnear, float& tfar)
	{
		using namespace glm;

		// fast slab method

		// where the ray intersects this line
		// O + tD = Bmin
		// O + tD = Bmax
		// tMin = (Bmin - O) / D
		// tMax = (Bmax - O) / D
		vec3 tMin = (_min - start) * invDir;
		vec3 tMax = (_max - start) * invDir;

		vec3 t1 = glm::min(tMin, tMax);
		vec3 t2 = glm::max(tMin, tMax);
		tnear = glm::max(glm::max(t1.x, t1.y), t1.z);
		tfar  = glm::min(glm::min(t2.x, t2.y), t2.z);

		return tfar >= tnear;
	}
	Bounds Bounds::Transform(const glm::mat4& trans) const
	{
		Bounds b{};
		auto corners = GetHomogenousCorners();
		b._min = glm::vec3(trans * corners[0]);
		b._max = b._min;
		for (auto i = 1; i < 8; ++i)
		{
			auto const tc = glm::vec3(trans * corners[i]);
			b << tc;
		}

		return b;
	}
	glm::vec3 Bounds::operator[](size_t index) const
	{
		assert(index < 2);
		return (&_min)[index];
	}
	glm::vec3& Bounds::operator[](size_t index)
	{
		assert(index < 2);
		return (&_min)[index];
	}
	glm::vec3& Bounds::min()
	{
		return _min;
	}
	glm::vec3& Bounds::max()
	{
		return _max;
	}
	const glm::vec3& Bounds::min() const
	{
		return _min;
	}
	const glm::vec3& Bounds::max() const
	{
		return _max;
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
		return _min == other._min && _max == other._max;
	}
	bool Bounds::operator!=(const Bounds& other) const
	{
		return !(operator==(other));
	}
	Bounds Bounds::operator+(const Bounds& other) const
	{
		Bounds result{};
		result._min = glm::min(_min, other._min);
		result._max = glm::max(_max, other._max);

		return result;
	}
	bool Bounds::Empty() const
	{
		return _min.x == std::numeric_limits<float>::max();
	}
}