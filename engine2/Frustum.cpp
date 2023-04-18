#include "./Frustum.h"
#include "./Bounds.h"

namespace jsr {
	Frustum::Frustum() : planes()
	{
	}
	Frustum::Frustum(const glm::mat4& projection, const glm::mat4& modelview)
	{
		glm::mat4 M = projection * glm::transpose(modelview);
		// Left clipping plane
		planes[0].x = M[4][1] + M[1][1];
		planes[0].y = M[4][2] + M[1][2];
		planes[0].z = M[4][3] + M[1][3];
		planes[0].w = M[4][4] + M[1][4];

		// Right clipping plane
		planes[1].x = M[4][1] - M[1][1];
		planes[1].y = M[4][2] - M[1][2];
		planes[1].z = M[4][3] - M[1][3];
		planes[1].w = M[4][4] - M[1][4];

		// Top clipping plane
		planes[2].x = M[4][1] - M[2][1];
		planes[2].y = M[4][2] - M[2][2];
		planes[2].z = M[4][3] - M[2][3];
		planes[2].w = M[4][4] - M[2][4];

		// Bottom clipping plane
		planes[3].x = M[4][1] + M[2][1];
		planes[3].y = M[4][2] + M[2][2];
		planes[3].z = M[4][3] + M[2][3];
		planes[3].w = M[4][4] + M[2][4];

		// Near clipping plane
		planes[4].x = M[4][1] + M[3][1];
		planes[4].y = M[4][2] + M[3][2];
		planes[4].z = M[4][3] + M[3][3];
		planes[4].w = M[4][4] + M[3][4];

		// Far clipping plane
		planes[5].x = M[4][1] - M[3][1];
		planes[5].y = M[4][2] - M[3][2];
		planes[5].z = M[4][3] - M[3][3];
		planes[5].w = M[4][4] - M[3][4];
		
	}
	bool Frustum::BoundsTest(const Bounds& b) const
	{
		if (!SphereTest(b.GetCenter(), b.GetRadius()))
		{
			return false;
		}

		glm::vec3 corners[8];
		b.GetCorners(corners);

		for (int i = 0; i < 8; ++i)
		{
			for (int k = 0; k < 6; ++k)
			{
				float dist = glm::dot(corners[i], glm::vec3(planes[k])) + planes[k].w;
				if (dist >= 0)
				{
					return true;
				}
			}
		}
		return false;
	}
	bool Frustum::SphereTest(const glm::vec3& p, float radius) const
	{
		for (int i = 0; i < 6; i++)
		{
			float dist = glm::dot(p, glm::vec3(planes[i])) + planes[i].w + radius;
			if (dist < 0) return false; // sphere culled
		}
		return true;
	}
}