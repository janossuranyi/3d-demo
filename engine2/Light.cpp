#include <string>

#include "./Light.h"
#include "./Material.h"
#include "./Node3D.h"

namespace jsr {

	Light::Light() : Light(LIGHT_POINT) {}
	Light::Light(eLightType type) : Light(type, nullptr) {}
	Light::Light(eLightType type, Node3D* pNode) : Light(type, pNode, nullptr) {}
	Light::Light(eLightType type_, Node3D* pNode, Material const* pMaterial) :
		type(type_),
		node(pNode),
		shader(pMaterial),
		radius(0.0f){}

	Light& Light::SetNode(Node3D* pNode)
	{
		node = pNode;
		return *this;
	}

	Light& Light::SetMaterial(Material const* pMaterial)
	{
		shader = pMaterial;
		return *this;
	}

	Node3D* Light::GetNode() const
	{
		return node;
	}

	eLightType Light::GetType() const
	{
		return type;
	}

	Material const* Light::GetMaterial() const
	{
		return shader;
	}

	void Light::SetId(int id)
	{
		this->id = id;
	}

	void Light::SetName(const std::string& name)
	{
		this->name = name;
	}

	const std::string& Light::GetName() const
	{
		return name;
	}

}