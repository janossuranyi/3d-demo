#pragma once

#include <vector>
#include "./Math.h"
#include "./Model.h"
#include "./Light.h"

namespace jsr {

	class Entity3D;
	class Node3D
	{
	public:
		Node3D();
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
		void				SetParent(Node3D* parent);
		void				AddModel(RenderModel* model);
		void				AddLight(Light* model);
		int					GetNumChildren() const;
		int					GetNumEntities() const;
		const Entity3D*		GetEntities() const;
		const Node3D* const* GetChildren() const;
	private:
		void				Changed();
		Node3D* pParent;
		glm::mat4 localToWorldMatrix;
		glm::vec3 origin;
		glm::vec3 scale;
		glm::quat dir;
		std::vector<Node3D*> children;
		std::vector<Entity3D> entities;
		bool changed;
	};

}

