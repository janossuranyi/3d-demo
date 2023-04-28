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
		shader(pMaterial) {
	
		switch (type)
		{
		case LIGHT_POINT:
			break;
		}
	}

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

	float Light::GetBounds() const
	{
		return opts.radius * (glm::sqrt(opts.color.power * opts.invAttnCutoff) - 1.0);
	}

}