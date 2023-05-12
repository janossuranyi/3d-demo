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
		planes[0].x = M[3][0] + M[0][0];
		planes[0].y = M[3][1] + M[0][1];
		planes[0].z = M[3][2] + M[0][2];
		planes[0].w = M[3][3] + M[0][3];

		// Right clipping plane
		planes[1].x = M[3][0] - M[0][0];
		planes[1].y = M[3][1] - M[0][1];
		planes[1].z = M[3][2] - M[0][2];
		planes[1].w = M[3][3] - M[0][3];

		// Top clipping plane
		planes[2].x = M[3][0] - M[1][0];
		planes[2].y = M[3][1] - M[1][1];
		planes[2].z = M[3][2] - M[1][2];
		planes[2].w = M[3][3] - M[1][3];

		// Bottom clipping plane
		planes[3].x = M[3][0] + M[1][0];
		planes[3].y = M[3][1] + M[1][1];
		planes[3].z = M[3][2] + M[1][2];
		planes[3].w = M[3][3] + M[1][3];

		// Near clipping plane
		planes[4].x = M[3][0] + M[2][0];
		planes[4].y = M[3][1] + M[2][1];
		planes[4].z = M[3][2] + M[2][2];
		planes[4].w = M[3][3] + M[2][3];

		// Far clipping plane
		planes[5].x = M[3][0] - M[2][0];
		planes[5].y = M[3][1] - M[2][1];
		planes[5].z = M[3][2] - M[2][2];
		planes[5].w = M[3][3] - M[2][3];
		
	}
	bool Frustum::BoundsTest(const Bounds& b) const
	{
		if ( !SphereTest( b.GetSphere() ) )
		{
			return false;
		}

		auto corners = b.GetCorners();

		for (int i = 0; i < 8; ++i)
		{
			for (int k = 0; k < 6; ++k)
			{
				float const dist = glm::dot(corners[i], glm::vec3(planes[k])) + planes[k].w;
				if (dist >= 0)
				{
					return true;
				}
			}
		}
		return false;
	}
	bool Frustum::SphereTest(const Sphere& s) const
	{
		for (int i = 0; i < 6; ++i)
		{
			float dist = glm::dot(s.GetCenter(), glm::vec3(planes[i])) + planes[i].w + s.GetRadius();
			if (dist < 0) return false; // sphere culled
		}
		return true;
	}
}