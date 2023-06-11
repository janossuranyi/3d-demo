#pragma once

#include <vector>
#include "./Math.h"
#include "./Model.h"
#include "./Light.h"
#include "./Entity3D.h"

namespace jsr {

	class Node3D
	{
	public:
		Node3D();
		Node3D(const std::string& name);
		virtual ~Node3D();
		glm::quat const&	GetDir() const;
		glm::vec3 const&	GetOrigin() const;
		glm::vec3 const&	GetScale() const;
		Node3D*				GetParent();
		glm::mat4 const&	GetLocalToWorldMatrix();
		void				SetOrigin(glm::vec3 const& v);
		void				SetScale(glm::vec3 const& v);
		void				SetDir(glm::quat const& v);
		void				SetDir(float radX, float radY, float radZ);
		void				SetDir(const glm::vec3& angular);
		void				SetParent(Node3D* parent);
		int					GetNumChildren() const;
		int					GetNumEntities() const;
		Entity3D&			GetEntity();
		Entity3D const&		GetEntity() const;
		std::string			GetName() const;
		void				SetName(const std::string& name);
		Node3D**			GetChildren();
		void				AddChild(Node3D* child);
		unsigned int		viewCount;
	private:
		void				Changed();
		std::string name;
		Node3D* pParent;
		Entity3D entity;
		glm::mat4 localToWorldMatrix;
		glm::vec3 origin;
		glm::vec3 scale;
		glm::quat dir;
		std::vector<Node3D*> children;
		bool changed;
	};

}

