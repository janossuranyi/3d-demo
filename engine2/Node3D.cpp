#include "./Node3D.h"

using namespace glm;

namespace jsr {

	Node3D::Node3D() :
		pParent(),
		localToWorldMatrix(1.0f),
		origin(vec3(0.0f)),
		scale(vec3(1.0f)),
		changed(true),
		dir(1.0f,0.0f, 0.0f, 0.0f)
	{}

	Node3D::~Node3D()
	{
	}

	quat const& Node3D::GetDir() const
	{
		return dir;
	}

	vec3 const& Node3D::GetOrigin() const
	{
		return origin;
	}

	Node3D* Node3D::GetParent()
	{
		return pParent;
	}

	mat4 const& Node3D::GetLocalToWorldMatrix()
	{
		static mat4 IDENT(1.0f);

		if (changed)
		{
			changed = false;

			localToWorldMatrix = glm::translate(IDENT, origin) * mat4_cast(dir);
			localToWorldMatrix = glm::scale(localToWorldMatrix, scale);
						
			if (pParent)
			{
				localToWorldMatrix = pParent->GetLocalToWorldMatrix() * localToWorldMatrix;
			}
		}

		return localToWorldMatrix;
	}

	void Node3D::SetOrigin(glm::vec3 const& o)
	{
		origin = o;
		Changed();
	}

	void Node3D::SetScale(glm::vec3 const& v)
	{
		scale = v;
		Changed();
	}

	void Node3D::SetDir(glm::quat const& d)
	{
		dir = d;
		Changed();
	}

	void Node3D::SetDir(float radX, float radY, float radZ)
	{
		dir = glm::angleAxis(radX, vec3(1.0f, 0.0f, 0.0f));
		dir *= glm::angleAxis(radY, vec3(0.0f, 1.0f, 0.0f));
		dir *= glm::angleAxis(radZ, vec3(0.0f, 0.0f, 1.0f));
	}

	void Node3D::SetParent(Node3D* parent)
	{
		pParent = parent;
		Changed();
	}

	void Node3D::Changed()
	{
		changed = true;
		for (auto* node : children)
		{
			node->Changed();
		}
	}
	
}