#pragma once

#include <vector>
#include "./Math.h"

namespace jsr {

	class Entity3D;
	class Node3D
	{
	public:
		Node3D();
		virtual ~Node3D();
		glm::quat const&	GetDir() const;
		glm::vec3 const&	GetOrigin() const;
		Node3D*				GetParent();
		glm::mat4 const&	GetLocalToWorldMatrix();
		void				SetOrigin(glm::vec3 const& o);
		void				SetDir(glm::quat const& d);
		void				SetDir(float radX, float radY, float radZ);
		void				SetParent(Node3D* parent);
	private:
		Node3D* pParent;
		glm::mat4 localToWorldMatrix;
		glm::vec3 origin;
		glm::quat dir;
		std::vector<Node3D*> children;
		std::vector<Entity3D*> entities;
		bool changed;
	};

}

