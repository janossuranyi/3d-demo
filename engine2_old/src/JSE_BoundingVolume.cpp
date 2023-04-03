#include "JSE.h"

namespace js {
	BoundingSphere::BoundingSphere(const vec3& centroid, float radius)
	{
		_center = centroid;
		_radius = radius;
		_radius2 = radius * radius;
	}

	bool BoundingSphere::contains(const vec3& p)
	{
		const vec3 c = p - _center;
		const float distance2 = dot(c, c);

		return distance2 <= _radius2;
	}

	float BoundingSphere::pointDistance(const vec3& p)
	{
		const float distance = length(p - _center);

		return distance - _radius;
	}

	bool BoundingSphere::rayIntersect(const vec3& p0, const vec3& v, float& t1, float& t2)
	{
		const vec3 u = normalize(v);
		const vec3 oc = p0 - _center;
		const float aoc = length(oc);
		const float aoc2 = aoc * aoc;
		const float r2 = _radius2;
		const float k = dot(u, oc);
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

	bool BoundingSphere::lineIntersect(const vec3& p0, const vec3& p1, float& t1, float& t2)
	{
		const vec3 v = p1 - p0;

		if (rayIntersect(p0, v, t1, t2))
		{
			if (t1 < 0.0f || t2 < 0.0f)
			{
				// p0 is inside the sphere
				t1 = 0.0f;
				t2 = 0.0f;
				return true;
			}

			const float l = length(v);
			if ((l - t1) >= 0.0f || (l - t2) >= 0.0f)
			{
				return true;
			}
		}

		return false;
	}

	bool BoundingSphere::intersect(const BoundingSphere& x) const
	{
		const vec3 c = x._center - _center;
		const float distance2 = dot(c, c);
		const float sr = x._radius + _radius;

		return distance2 <= (sr * sr);
	}

	bool BoundingSphere::intersect(const BoundingBox& x) const
	{
		// get box closest point to sphere center by clamping
		const vec3 v = max(x.min(), min(_center, x.max()));
		const float d = dot(v, v);

		return d < _radius2;

	}

	vec3 BoundingSphere::center() const
	{
		return _center;
	}

	float BoundingSphere::radius() const
	{
		return _radius;
	}

	BoundingBox::BoundingBox(const vec3& v0, const vec3& v1)
	{
		minis = glm::min(v0, v1);
		maxis = glm::max(v0, v1);
	}

	void BoundingBox::merge(const BoundingBox& box)
	{
		minis = glm::min(minis, box.minis);
		maxis = glm::max(maxis, box.maxis);
	}

	void BoundingBox::merge(const vec3& p)
	{
		minis = glm::min(minis, p);
		maxis = glm::max(maxis, p);
	}

	BoundingSphere BoundingBox::getSphere() const
	{
		const vec3 center = 0.5f * (minis + maxis);
		const float radius = length(maxis - center);

		return { center,radius };
	}

	vec3 BoundingBox::getSize() const
	{
		return maxis - minis;
	}

	std::vector<vec3> BoundingBox::getCorners() const {
		return {
			vec3(minis[0], minis[1], minis[2]),
			vec3(minis[0], minis[1], maxis[2]),
			vec3(minis[0], maxis[1], minis[2]),
			vec3(minis[0], maxis[1], maxis[2]),
			vec3(maxis[0], minis[1], minis[2]),
			vec3(maxis[0], minis[1], maxis[2]),
			vec3(maxis[0], maxis[1], minis[2]),
			vec3(maxis[0], maxis[1], maxis[2]) };
	}

	std::vector<vec4> BoundingBox::getHomogeneousCorners() const {
		return {
			vec4(minis[0], minis[1], minis[2], 1.0f),
			vec4(minis[0], minis[1], maxis[2], 1.0f),
			vec4(minis[0], maxis[1], minis[2], 1.0f),
			vec4(minis[0], maxis[1], maxis[2], 1.0f),
			vec4(maxis[0], minis[1], minis[2], 1.0f),
			vec4(maxis[0], minis[1], maxis[2], 1.0f),
			vec4(maxis[0], maxis[1], minis[2], 1.0f),
			vec4(maxis[0], maxis[1], maxis[2], 1.0f) };
	}

	vec3 BoundingBox::getCenter() const
	{
		return 0.5f * (minis + maxis);
	}

	BoundingBox BoundingBox::transformed(const mat4& trans) const {
		BoundingBox newBox;
		const std::vector<vec4> corners = getHomogeneousCorners();

		newBox.minis = vec3(trans * corners[0]);
		newBox.maxis = newBox.minis;
		for (size_t i = 1; i < 8; ++i) {
			const vec3 transformedCorner = vec3(trans * corners[i]);
			newBox.minis = glm::min(newBox.minis, transformedCorner);
			newBox.maxis = glm::max(newBox.maxis, transformedCorner);
		}
		return newBox;
	}

	bool BoundingBox::contains(const vec3& point) const
	{
		return all(greaterThanEqual(point, minis)) && all(lessThanEqual(point, maxis));
	}

	bool BoundingBox::empty() const
	{
		return minis[0] == std::numeric_limits<float>::max();
	}

	vec3 BoundingBox::getPositiveVertex(const vec3& normal) const
	{
		vec3 positiveVertex = minis;

		if (normal.x >= 0.0f) positiveVertex.x = maxis.x;
		if (normal.y >= 0.0f) positiveVertex.y = maxis.y;
		if (normal.z >= 0.0f) positiveVertex.z = maxis.z;

		return positiveVertex;
	}

	vec3 BoundingBox::getNegativeVertex(const vec3& normal) const
	{
		vec3 negativeVertex = maxis;

		if (normal.x >= 0.0f) negativeVertex.x = minis.x;
		if (normal.y >= 0.0f) negativeVertex.y = minis.y;
		if (normal.z >= 0.0f) negativeVertex.z = minis.z;

		return negativeVertex;
	}

	vec3 BoundingBox::min() const
	{
		return minis;
	}

	vec3 BoundingBox::max() const
	{
		return maxis;
	}

	bool BoundingBox::intersect(const vec3& rayOrigin, const vec3& rayDir, vec2& result) const
	{
		const vec3 tMin = (minis - rayOrigin) / rayDir;
		const vec3 tMax = (maxis - rayOrigin) / rayDir;
		const vec3 t1 = glm::min(tMin, tMax);
		const vec3 t2 = glm::max(tMin, tMax);

		float tNear = glm::max(glm::max(t1.x, t1.y), t1.z);
		float tFar = glm::min(glm::min(t2.x, t2.y), t2.z);

		result = vec2(tNear, tFar);

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

	JseFrustum::JseFrustum(const mat4& vp)
	{
		// We have to access rows easily, so transpose.
		const mat4 tvp = transpose(vp);
		const mat4 ivp = inverse(vp);
		// Based on Fast Extraction of Viewing JseFrustum Planes from the World- View-Projection Matrix, G. Gribb, K. Hartmann
		// (https://www.gamedevs.org/uploads/fast-extraction-viewing-Frustum-planes-from-world-view-projection-matrix.pdf)
		_planes[LEFT] = tvp[3] + tvp[0];
		_planes[RIGHT] = tvp[3] - tvp[0];
		_planes[TOP] = tvp[3] - tvp[1];
		_planes[BOTTOM] = tvp[3] + tvp[1];
		_planes[NEAR] = tvp[2];
		_planes[FAR] = tvp[3] - tvp[2];

		for (int i = 0; i < 6; ++i)
		{
			_planes[i] = normalize(_planes[i]);
		}

		// Reproject the 8 corners of the JseFrustum from NDC to world space.
		static const std::array<vec4, 8> ndcCorner = {
			vec4(-1.0f, -1.0f, 0.0f, 1.0f),
			vec4(-1.0f, -1.0f, 1.0f, 1.0f),
			vec4(-1.0f,  1.0f, 0.0f, 1.0f),
			vec4(-1.0f,  1.0f, 1.0f, 1.0f),
			vec4(1.0f, -1.0f, 0.0f, 1.0f),
			vec4(1.0f, -1.0f, 1.0f, 1.0f),
			vec4(1.0f,  1.0f, 0.0f, 1.0f),
			vec4(1.0f,  1.0f, 1.0f, 1.0f) };

		for (uint32_t i = 0; i < 8; ++i) {
			const vec4 corn = ivp * ndcCorner[i];
			_corners[i] = vec3(corn) / corn[3];
		}
	}

	bool JseFrustum::intersects_v1(const BoundingBox& box) const
	{
		const std::vector<vec4> corners = box.getHomogeneousCorners();
		// For each of the JseFrustum planes, check if all points are in the "outside" half-space.
		for (uint32_t pid = 0; pid < JseFrustumPlane::COUNT; ++pid) {
			if ((dot(_planes[pid], corners[0]) < 0.0) &&
				(dot(_planes[pid], corners[1]) < 0.0) &&
				(dot(_planes[pid], corners[2]) < 0.0) &&
				(dot(_planes[pid], corners[3]) < 0.0) &&
				(dot(_planes[pid], corners[4]) < 0.0) &&
				(dot(_planes[pid], corners[5]) < 0.0) &&
				(dot(_planes[pid], corners[6]) < 0.0) &&
				(dot(_planes[pid], corners[7]) < 0.0)) {
				return false;
			}
		}
		/// \todo Implement JseFrustum corner checks to weed out more false positives.
		return true;
	}

	bool JseFrustum::intersects_v2(const BoundingBox& box) const
	{
		bool result = true; // inside

		for (int i = 0; i < 6; i++)
		{
			const float pos = _planes[i].w;
			const vec3 normal = vec3(_planes[i]);

			if (dot(normal, box.getPositiveVertex(normal)) + pos < 0.0f)
			{
				return false;	// outside
			}

			if (dot(normal, box.getNegativeVertex(normal)) + pos < 0.0f)
			{
				result = true;	// intersect
			}
		}

		return result;
	}

	bool JseFrustum::intersect(const vec3& point) const
	{
		bool result = true;	// TEST_INSIDE;

		for (int i = 0; i < 6; i++)
		{
			const float pos = _planes[i].w;
			const vec3 normal = vec3(_planes[i]);

			if (dot(normal, point) + pos < 0.0f)
			{
				return false;	// TEST_OUTSIDE;
			}
		}

		return result;
	}

	bool JseFrustum::intersect(const BoundingSphere& sphere) const
	{
		bool result = true;	// TEST_INSIDE;

		for (int i = 0; i < 6; i++)
		{
			const float pos = _planes[i].w;
			const vec3 normal = vec3(_planes[i]);

			if (dot(normal, sphere.center()) + pos + sphere.radius() < 0.0f)
			{
				return false;	// TEST_OUTSIDE;
			}
		}

		return result;
	}
}