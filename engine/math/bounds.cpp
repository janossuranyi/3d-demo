#include "math/bounds.h"

namespace math {
	BoundingSphere::BoundingSphere(const glm::vec3& centroid, float radius)
	{
		_center = centroid;
		_radius = radius;
		_radius2 = radius * radius;
	}

	bool BoundingSphere::contains(const glm::vec3& p)
	{
		const glm::vec3 c = p - _center;
		const float distance2 = glm::dot(c, c);

		return distance2 <= _radius2;
	}

	float BoundingSphere::pointDistance(const glm::vec3& p)
	{
		const float distance = glm::length(p - _center);

		return distance - _radius;
	}

	bool BoundingSphere::rayIntersect(const glm::vec3& p0, const glm::vec3& v, float& t1, float& t2)
	{
		const glm::vec3 u = glm::normalize(v);
		const glm::vec3 oc = p0 - _center;
		const float aoc = glm::length(oc);
		const float aoc2 = aoc * aoc;
		const float r2 = _radius2;
		const float k = glm::dot(u, oc);
		float K = k * k - (aoc2 - r2);

		if (K < 0) return false;

		const float d = -k;
		if (K < epsilon)
		{
			t1 = d;
			t2 = 0.0f;
			return true;
		}

		const float Ksr = std::sqrtf(K);
		t1 = (d - Ksr);
		t2 = (d + Ksr);

		return true;
	}

	bool BoundingSphere::lineIntersect(const glm::vec3& p0, const glm::vec3& p1, float& t1, float& t2)
	{
		const glm::vec3 v = p1 - p0;

		if (rayIntersect(p0, v, t1, t2))
		{
			if (t1 < 0.0f || t2 < 0.0f)
			{
				// p0 is inside the sphere
				t1 = 0.0f;
				t2 = 0.0f;
				return true;
			}

			const float l = glm::length(v);
			if ((l - t1) >= 0.0f || (l - t2) >= 0.0f)
			{
				return true;
			}
		}

		return false;
	}

	bool BoundingSphere::intersect(const BoundingSphere& x) const
	{
		const glm::vec3 c = x._center - _center;
		const float distance2 = glm::dot(c, c);
		const float sr = x._radius + _radius;

		return distance2 <= (sr * sr);
	}

	bool BoundingSphere::intersect(const BoundingBox& x) const
	{
		// get box closest point to sphere center by clamping
		const glm::vec3 v = glm::max(x.min(), glm::min(_center, x.max()));
		const float d = glm::dot(v, v);

		return d < _radius2;

	}

	glm::vec3 BoundingSphere::center() const
	{
		return _center;
	}

	float BoundingSphere::radius() const
	{
		return _radius;
	}

	BoundingBox::BoundingBox(const glm::vec3& v0, const glm::vec3& v1)
	{
		minis = glm::min(v0, v1);
		maxis = glm::max(v0, v1);
	}

	void BoundingBox::merge(const BoundingBox& box)
	{
		minis = glm::min(minis, box.minis);
		maxis = glm::max(maxis, box.maxis);
	}

	void BoundingBox::merge(const glm::vec3& p)
	{
		minis = glm::min(minis, p);
		maxis = glm::max(maxis, p);
	}

	BoundingSphere BoundingBox::getSphere() const
	{
		const glm::vec3 center = 0.5f * (minis + maxis);
		const float radius = glm::length(maxis - center);

		return { center,radius };
	}

	glm::vec3 BoundingBox::getSize() const
	{
		return maxis - minis;
	}

	std::vector<glm::vec3> BoundingBox::getCorners() const {
		return {
			glm::vec3(minis[0], minis[1], minis[2]),
			glm::vec3(minis[0], minis[1], maxis[2]),
			glm::vec3(minis[0], maxis[1], minis[2]),
			glm::vec3(minis[0], maxis[1], maxis[2]),
			glm::vec3(maxis[0], minis[1], minis[2]),
			glm::vec3(maxis[0], minis[1], maxis[2]),
			glm::vec3(maxis[0], maxis[1], minis[2]),
			glm::vec3(maxis[0], maxis[1], maxis[2]) };
	}

	std::vector<glm::vec4> BoundingBox::getHomogeneousCorners() const {
		return {
			glm::vec4(minis[0], minis[1], minis[2], 1.0f),
			glm::vec4(minis[0], minis[1], maxis[2], 1.0f),
			glm::vec4(minis[0], maxis[1], minis[2], 1.0f),
			glm::vec4(minis[0], maxis[1], maxis[2], 1.0f),
			glm::vec4(maxis[0], minis[1], minis[2], 1.0f),
			glm::vec4(maxis[0], minis[1], maxis[2], 1.0f),
			glm::vec4(maxis[0], maxis[1], minis[2], 1.0f),
			glm::vec4(maxis[0], maxis[1], maxis[2], 1.0f) };
	}

	glm::vec3 BoundingBox::getCenter() const
	{
		return 0.5f * (minis + maxis);
	}

	BoundingBox BoundingBox::transformed(const glm::mat4& trans) const {
		BoundingBox newBox;
		const std::vector<glm::vec4> corners = getHomogeneousCorners();

		newBox.minis = glm::vec3(trans * corners[0]);
		newBox.maxis = newBox.minis;
		for (size_t i = 1; i < 8; ++i) {
			const glm::vec3 transformedCorner = glm::vec3(trans * corners[i]);
			newBox.minis = glm::min(newBox.minis, transformedCorner);
			newBox.maxis = glm::max(newBox.maxis, transformedCorner);
		}
		return newBox;
	}

	bool BoundingBox::contains(const glm::vec3& point) const
	{
		return glm::all(glm::greaterThanEqual(point, minis)) && glm::all(glm::lessThanEqual(point, maxis));
	}

	bool BoundingBox::empty() const
	{
		return minis[0] == std::numeric_limits<float>::max();
	}

	glm::vec3 BoundingBox::getPositiveVertex(const glm::vec3& normal) const
	{
		glm::vec3 positiveVertex = minis;

		if (normal.x >= 0.0f) positiveVertex.x = maxis.x;
		if (normal.y >= 0.0f) positiveVertex.y = maxis.y;
		if (normal.z >= 0.0f) positiveVertex.z = maxis.z;

		return positiveVertex;
	}

	glm::vec3 BoundingBox::getNegativeVertex(const glm::vec3& normal) const
	{
		glm::vec3 negativeVertex = maxis;

		if (normal.x >= 0.0f) negativeVertex.x = minis.x;
		if (normal.y >= 0.0f) negativeVertex.y = minis.y;
		if (normal.z >= 0.0f) negativeVertex.z = minis.z;

		return negativeVertex;
	}

	glm::vec3 BoundingBox::min() const
	{
		return minis;
	}

	glm::vec3 BoundingBox::max() const
	{
		return maxis;
	}

	bool BoundingBox::intersect(const glm::vec3& rayOrigin, const glm::vec3& rayDir, glm::vec2& result) const
	{
		const glm::vec3 tMin = (minis - rayOrigin) / rayDir;
		const glm::vec3 tMax = (maxis - rayOrigin) / rayDir;
		const glm::vec3 t1 = glm::min(tMin, tMax);
		const glm::vec3 t2 = glm::max(tMin, tMax);

		float tNear = glm::max(glm::max(t1.x, t1.y), t1.z);
		float tFar = glm::min(glm::min(t2.x, t2.y), t2.z);

		result = glm::vec2(tNear, tFar);

		return (tNear <= tFar);
	};

	bool BoundingBox::intersect(const BoundingBox& b) const
	{
		// SIMD optimized AABB-AABB test
		// Optimized by removing conditional branches
		const bool x = minis.x <= b.maxis.x && maxis.x >= minis.x;
		const bool y = minis.y <= b.maxis.y && maxis.y >= minis.y;
		const bool z = minis.z <= b.maxis.z && maxis.z >= minis.z;

		return x && y && z;
	}

	bool BoundingBox::intersect(const BoundingSphere& x) const
	{
		return x.intersect(*this);
	}

	Frustum::Frustum(const glm::mat4& vp)
	{
		// We have to access rows easily, so transpose.
		const glm::mat4 tvp = glm::transpose(vp);
		const glm::mat4 ivp = glm::inverse(vp);
		// Based on Fast Extraction of Viewing Frustum Planes from the World- View-Projection Matrix, G. Gribb, K. Hartmann
		// (https://www.gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf)
		_planes[LEFT] = tvp[3] + tvp[0];
		_planes[RIGHT] = tvp[3] - tvp[0];
		_planes[TOP] = tvp[3] - tvp[1];
		_planes[BOTTOM] = tvp[3] + tvp[1];
		_planes[NEAR] = tvp[2];
		_planes[FAR] = tvp[3] - tvp[2];

		for (int i = 0; i < 6; ++i)
		{
			_planes[i] = glm::normalize(_planes[i]);
		}

		// Reproject the 8 corners of the frustum from NDC to world space.
		static const std::array<glm::vec4, 8> ndcCorner = {
			glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f),
			glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f),
			glm::vec4(-1.0f,  1.0f, 0.0f, 1.0f),
			glm::vec4(-1.0f,  1.0f, 1.0f, 1.0f),
			glm::vec4(1.0f, -1.0f, 0.0f, 1.0f),
			glm::vec4(1.0f, -1.0f, 1.0f, 1.0f),
			glm::vec4(1.0f,  1.0f, 0.0f, 1.0f),
			glm::vec4(1.0f,  1.0f, 1.0f, 1.0f) };

		for (uint i = 0; i < 8; ++i) {
			const glm::vec4 corn = ivp * ndcCorner[i];
			_corners[i] = glm::vec3(corn) / corn[3];
		}
	}

	bool Frustum::intersects_v1(const BoundingBox& box) const
	{
		const std::vector<glm::vec4> corners = box.getHomogeneousCorners();
		// For each of the frustum planes, check if all points are in the "outside" half-space.
		for (uint pid = 0; pid < FrustumPlane::COUNT; ++pid) {
			if ((glm::dot(_planes[pid], corners[0]) < 0.0) &&
				(glm::dot(_planes[pid], corners[1]) < 0.0) &&
				(glm::dot(_planes[pid], corners[2]) < 0.0) &&
				(glm::dot(_planes[pid], corners[3]) < 0.0) &&
				(glm::dot(_planes[pid], corners[4]) < 0.0) &&
				(glm::dot(_planes[pid], corners[5]) < 0.0) &&
				(glm::dot(_planes[pid], corners[6]) < 0.0) &&
				(glm::dot(_planes[pid], corners[7]) < 0.0)) {
				return false;
			}
		}
		/// \todo Implement frustum corner checks to weed out more false positives.
		return true;
	}

	bool Frustum::intersects_v2(const BoundingBox& box) const
	{
		bool result = true; // inside

		for (int i = 0; i < 6; i++)
		{
			const float pos = _planes[i].w;
			const glm::vec3 normal = glm::vec3(_planes[i]);

			if (glm::dot(normal, box.getPositiveVertex(normal)) + pos < 0.0f)
			{
				return false;	// outside
			}

			if (glm::dot(normal, box.getNegativeVertex(normal)) + pos < 0.0f)
			{
				result = true;	// intersect
			}
		}

		return result;
	}

	bool Frustum::intersect(const glm::vec3& point) const
	{
		bool result = true;	// TEST_INSIDE;

		for (int i = 0; i < 6; i++)
		{
			const float pos = _planes[i].w;
			const glm::vec3 normal = glm::vec3(_planes[i]);

			if (glm::dot(normal, point) + pos < 0.0f)
			{
				return false;	// TEST_OUTSIDE;
			}
		}

		return result;
	}

	bool Frustum::intersect(const BoundingSphere& sphere) const
	{
		bool result = true;	// TEST_INSIDE;

		for (int i = 0; i < 6; i++)
		{
			const float pos = _planes[i].w;
			const glm::vec3 normal = glm::vec3(_planes[i]);

			if (glm::dot(normal, sphere.center()) + pos + sphere.radius() < 0.0f)
			{
				return false;	// TEST_OUTSIDE;
			}
		}

		return result;
	}

}