#pragma once

#include <vector>
#include <memory>
#include "types.h"

class World;

class Entity3D
{
public:
	enum Type { MESH, LIGHT, CAMERA, TRANSFORM };
	using Ptr = std::shared_ptr<Entity3D>;

	Entity3D(Type type, World& world) :
		m_type(type),
		m_parent(-1),
		m_children(),
		m_worldPosition(),
		m_worldScale(),
		m_worldRotation(),
		m_world(world),
		m_mtxWorld(),
		m_id(-1) {}

	inline Type type() { return m_type; }
	bool addChild(int id_);
	Entity3D& translate(vec3& vec);
	Entity3D& rotate(vec3& eulerAngles);
	Entity3D& scale(vec3& vec);
	const mat4& worldMatrix() const { return m_mtxWorld; }
	void updatMatrix();
	void updateWorldMatrix();
	void saveWorldTransform();
	int id() const { return m_id; }
	void setId(int id) { m_id = id; }
private:
	int m_id;

	World& m_world;

	Type m_type;
	int m_parent;
	std::vector<int> m_children;

	vec3 m_worldPosition;
	vec3 m_worldScale;
	quat m_worldRotation;
	mat4 m_mtxWorld;

	vec3 m_savedPosition{};
	vec3 m_savedScale{};
	quat m_savedRotation{};
};