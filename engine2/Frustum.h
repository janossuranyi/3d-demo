#pragma once
#include <glm/glm.hpp>
#include "./Bounds.h"

namespace jsr {

	struct plane_t
	{
		glm::vec3	normal;
		float		distance;

		plane_t() : normal(glm::vec3(0, 0, 1)), distance(0.0f) {}
		plane_t(const glm::vec3& n, float d) : normal(glm::normalize(n)), distance(d) {}
		float PointDistance(const glm::vec3& p) const { return glm::dot(normal, p) + distance; }
		glm::vec4 GetVec4() const { return glm::vec4(normal, distance); }
	};

	class Bounds;
	class Frustum
	{
	public:
		Frustum();
		Frustum(const glm::mat4& projection, const glm::mat4& modelview);
		bool SphereTest(const Sphere& s) const;
		bool BoundsTest(const Bounds& b) const;
	private:
		glm::vec4	planes[6];
	};
}
