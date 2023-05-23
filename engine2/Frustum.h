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
		Frustum(const glm::mat4& vp);
		bool Intersects(const Bounds& box) const;
		bool Intersects2(const Bounds& box) const;
	private:
		glm::vec4	planes[6];
		glm::vec3	corners[8];
	};
}
