#include <cassert>
#include "entity3d.h"
#include "world.h"
#include "types.h"

void Entity3D::updateWorldMatrix()
{
	if (m_parent == -1) return;
	m_mtxWorld *= m_world.getEntity(m_parent).worldMatrix();
}

void Entity3D::setWorldMatrix(const mat4& mtx)
{
	m_mtxWorld = mtx;
	updateWorldMatrix();
}

bool Entity3D::addChild(int id_)
{
	for (int const e : m_children)
	{
		if (e == id_) return false;
	}

	m_children.push_back(id_);

	return true;
}

Entity3D& Entity3D::translate(vec3& vec)
{
	m_worldPosition = vec;

	return *this;
}

Entity3D& Entity3D::rotate(vec3& eulerAngles)
{
	quat qx = glm::angleAxis(eulerAngles.x, vec3(1, 0, 0));
	quat qy = glm::angleAxis(eulerAngles.y, vec3(0, 1, 0));
	quat qz = glm::angleAxis(eulerAngles.z, vec3(0, 0, 1));

	m_worldRotation = qx * qy * qz;

	return *this;
}

Entity3D& Entity3D::rotate(quat& quaternion)
{
	m_worldRotation = quaternion;

	return *this;
}

Entity3D& Entity3D::scale(vec3& vec)
{
	m_worldScale = vec;
	return *this;
}

void Entity3D::updatMatrix()
{
	mat4 m_W(1.0f);
	m_W = glm::translate(m_W, m_worldPosition);
	m_W = m_W * glm::mat4_cast(m_worldRotation);
	m_W = glm::scale(m_W, m_worldScale);

	m_mtxWorld = m_W;

	if (m_parent >= 0) updateWorldMatrix();

	if (m_children.empty() == false)
	{
		for (int const e : m_children)
		{
			Entity3D& ent = m_world.getEntity(e);
			ent.updateWorldMatrix();
		}
	}
}

void Entity3D::saveWorldTransform()
{
	m_savedPosition = m_worldPosition;
	m_savedRotation = m_worldRotation;
	m_savedScale = m_worldScale;
}

void Entity3D::updateParentChild()
{
	for (const int id_ : m_children)
	{
		Entity3D& ent = m_world.getEntity(id_);
		ent.m_parent = id();
	}
}
