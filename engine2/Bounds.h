#pragma once
#include <glm\glm.hpp>

namespace jsr {

	enum eIntersectionTestResult
	{
		IR_OUTSIDE = -1,
		IR_INTERSECT = 0,
		IR_INSIDE = 1
	};

	class Bounds
	{
	public:
		Bounds();
		Bounds(float _min, float _max);
		Bounds(float _min, float _max, const glm::vec3& _origin);
		Bounds(const glm::vec3& _min, const glm::vec3& _max, const glm::vec3& _origin);
		Bounds& Extend(float k);
		Bounds& Extend(const glm::vec3& v);
		Bounds& Extend(const Bounds& other);
		int Intersect(float fX, float fY, float fZ) const;
		int Intersect(const glm::vec3& v) const;
		float GetRadius() const;
		glm::vec3 GetOrigin() const;
		glm::vec3 GetMin() const;
		glm::vec3 GetMax() const;
		void GetCorners(glm::vec3 v[8]) const;
		/*
		compute the near and far intersections of the cube(stored in the x and y components) using the slab method
		no intersection means vec.x > vec.y (really tNear > tFar)
		*/
		glm::vec2 IntersectRay(const glm::vec3& rOrigin, const glm::vec3& rDir);
		bool operator==(const Bounds& other) const;
		bool operator!=(const Bounds& other) const;
		Bounds operator+(const Bounds& other) const;
		Bounds& operator+=(const Bounds& other);
	private:
		glm::vec3 min;
		glm::vec3 max;
		glm::vec3 origin;
	};
}