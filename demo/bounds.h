#pragma once

#include "common.h"
#include <array>

class BoundingSphere
{
public:
	BoundingSphere() = default;

	BoundingSphere(const glm::vec3& centroid, float radius);

	/* Checking that a given point is inside or outside of the sphere
	\param p a point in space
	\return 0=the point is on the perimeter, 1=inside, -1=outside
	*/
	bool		contains(const glm::vec3& p);

	/* Returns the distance of a point from the perimeter
	 \param p a point in space
	 \return the distance from the perimeter
	*/
	float		pointDistance(const glm::vec3& p);

	/* Finds an intersection between a ray and the sphere
	 \param p0 ray origin
	 \param v ray direction
	 \param t1 first intersection distance if exists
	 \param t2 second intersection distance if exists
	 \return true if intersection occured
	*/
	bool		rayIntersect(const glm::vec3& p0, const glm::vec3& v, float& t1, float& t2);

	/* Finds an intersection between a line segment and the sphere
	 \param p0 start point
	 \param p1 end point
	 \param t1 first intersection distance if exists
	 \param t2 second intersection distance if exists
	 \return true if intersection occured
	*/
	bool		lineIntersect(const glm::vec3& p0, const glm::vec3& p1, float& t1, float& t2);

	/* Decide if this sphere intersect with another
	 \param x sphere to test
	 \return true if intersection occured
	*/
	bool		intersect(const BoundingSphere& x) const;

	/* Decide if this sphere intersect with an AABB
	 \param x AABB to test
	 \return true if intersection occured
	*/
	bool		intersect(const BoundingBox& x) const;

	/* Returns the center of the sphere
	*/
	glm::vec3	center() const;

	/* Returns the radius of the sphere
	*/
	float		radius() const;

private:
	const float	epsilon = 1e-5f;
	glm::vec3	_center{ 0.0f,0.0f,0.0f };
	float		_radius{ 0.0f };
	float		_radius2{ 0.0f };
};


class BoundingBox
{
public:
	BoundingBox() = default;

	/* Corner based box constructor
	 \param v0 first corner
	 \param v1 second corner
	*/
	BoundingBox(const glm::vec3& v0, const glm::vec3& v1);

	/* Extends the current box. The result is the union of the two box
	 \param box Another bounding box
	*/
	void merge(const BoundingBox& box);

	/* Extends the current box by a point
	 \param p point to include
	*/
	void merge(const glm::vec3& p);

	/* Convert the box to a bounding sphere
	*/
	BoundingSphere getSphere() const;

	/* Query the size of the box	
	*/
	glm::vec3 getSize() const;

	/** Query the positions of the eight corners of the box, in the following order (with \p m=mini, \p M=maxi):
	 \p (m,m,m), \p (m,m,M), \p (m,M,m), \p (m,M,M), \p (M,m,m), \p (M,m,M), \p (M,M,m), \p (M,M,M)
	 \return a vector containing the box corners
	*/
	std::vector<glm::vec3> getCorners() const;

	/* Query the homogeneous positions of the eight corners of the box, in the following order (with \p m=mini, \p M=maxi):
	 \p (m,m,m,1), \p (m,m,M,1), \p (m,M,m,1), \p (m,M,M,1), \p (M,m,m,1), \p (M,m,M,1), \p (M,M,m,1), \p (M,M,M,1)
	 \return a vector containing the box corners
	 */
	std::vector<glm::vec4> getHomogeneousCorners() const;

	/* Get center of the box
	*/
	glm::vec3 getCenter() const;

	/* Compute the bounding box of the transformed current box
	 \param trans the transformation matrix
	 \return the bounding box of the transormed box
	*/
	BoundingBox transformed(const glm::mat4& trans) const;

	/* Indicates if a point is inside of the box
	 \param point point to test
	 \return true if the boundingf box contains the point
	*/
	bool contains(const glm::vec3& point) const;

	/* Indicates if the bounding box is emtpy
	 \return true if current bounding box is empty
	*/
	bool empty() const;

	/* compute the near and far intersections of the cube (stored in the x and y components) using the slab method.
	 No intersection means vec.x > vec.y (really tNear > tFar)
	 \param rayOrigin ray start position
	 \param rayDir ray direction
	 \return the near and far intersections, true if near <= far
	*/
	bool intersect(const glm::vec3& rayOrigin, const glm::vec3& rayDir, glm::vec2& result) const;

	/*
	 AABB vs. AABB intersection test
	 \param The another bounding box to test
	 \return true if the two box is overlapping
	*/
	bool intersect(const BoundingBox& b) const;

	/*
	 AABB vs. Sphere intersection test
	 \param The sphere to test
	 \return true if the two is overlapping
	*/
	bool intersect(const BoundingSphere& x) const;

	glm::vec3 getPositiveVertex(const glm::vec3& normal) const;
	glm::vec3 getNegativeVertex(const glm::vec3& normal) const;

	glm::vec3 min() const;
	glm::vec3 max() const;

private:
	glm::vec3 minis = glm::vec3(std::numeric_limits<float>::max());	///< Lower-back-left corner of the box.
	glm::vec3 maxis = glm::vec3(std::numeric_limits<float>::lowest()); ///< Higher-top-right corner of the box.

};

class Frustum
{
public:
	/**Create a frustum from a view-projection matrix.
	 \param vp the matrix defining the frustum
	 */
	Frustum(const glm::mat4& vp);

	/** Indicate if a bounding box intersect this frustum.
	\param box the bounding box to test
	\return true if the bounding box intersects the frustum.
	*/
	bool intersects_v1(const BoundingBox& box) const;
	bool intersects_v2(const BoundingBox& box) const;

	/** Indicate if a point intersect this frustum.
	\param point the bounding box to test
	\return true if point is inside the frustum.
	*/
	bool intersect(const glm::vec3& point) const;

	bool intersect(const BoundingSphere& sphere) const;
private:

	/** Helper enum for the frustum plane locations. */
	enum FrustumPlane : uint {
		LEFT = 0, RIGHT = 1, TOP = 2, BOTTOM = 3, NEAR = 4, FAR = 5, COUNT = 6
	};

	std::array<glm::vec4, FrustumPlane::COUNT> _planes; ///< Frustum hyperplane coefficients.
	std::array<glm::vec3, 8> _corners; ///< Frustum corners.

};