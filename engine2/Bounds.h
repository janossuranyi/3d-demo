#pragma once

#include <vector>
#include <glm\glm.hpp>

namespace jsr {

	static const float epsilon = 1e-5f;

	/*
	Implements an AABB
	*/
	class Bounds;
	class Sphere
	{
	public:
		Sphere() = default;
		Sphere(const glm::vec3& center, float r);
		bool Contains(const glm::vec3& p) const;
		float Distance(const glm::vec3& p) const;
		bool RayIntersect(const glm::vec3& start, const glm::vec3& dir, float& tnear, float& tfar) const;
		bool LineIntersect(const glm::vec3& start, const glm::vec3& end, float& tnear, float& tfar) const;
		bool Intersect(const Sphere& s) const; 
		bool Intersect(const Bounds& s) const;
		glm::vec3 GetCenter() const;
		float GetRadius() const;
	private:
		glm::vec3 center;
		float radius;
		float radius2;
	};

	class Bounds
	{
	public:
		Bounds();
		Bounds(const glm::vec3& a, const glm::vec3& b);
		Bounds& operator<<(const glm::vec3& v);
		Bounds& operator<<(const Bounds& other);
		bool Contains(const glm::vec3& p) const;
		float GetRadius() const;
		glm::vec3 GetCenter() const;
		glm::vec3 GetMin() const;
		glm::vec3 GetMax() const;
		glm::vec3 operator[](size_t index) const;
		glm::vec3& operator[](size_t index);
		glm::vec3& min();
		glm::vec3& max();
		const glm::vec3& min() const;
		const glm::vec3& max() const;
		std::vector<glm::vec3> GetCorners() const;
		std::vector<glm::vec4> GetHomogenousCorners() const;
		Sphere GetSphere() const;
		Bounds Transform(const glm::mat4& trans) const;
		/*
		compute the near and far intersections of the cube(stored in the x and y components) using the slab method
		no intersection means vec.x > vec.y (really tNear > tFar)
		*/
		bool RayIntersect(const glm::vec3& start, const glm::vec3& invdir, float& tmin, float& tmax);
		bool operator==(const Bounds& other) const;
		bool operator!=(const Bounds& other) const;
		Bounds operator+(const Bounds& other) const;
		bool Empty() const;
	private:
		glm::vec3 b[2];
	};
}