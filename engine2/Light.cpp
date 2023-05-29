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

	float Light::GetRadius() const
	{
		const auto r = -opts.linearAttn + glm::sqrt(opts.linearAttn * opts.linearAttn - 4.0f * opts.expAttn * (1.0f - 256.0f * opts.color.GetPower()));
		return r / (2.0f * opts.expAttn);
	}

	float Light::GetRadius2() const
	{
		const float INTENSITY_CUTOFF = 1.0f;
		const float ATTENUATION_CUTOFF = 1.0f / 256.0f;
		auto power = opts.color.GetPower() / (4.0f * glm::pi<float>());
		auto attenuation = glm::max(INTENSITY_CUTOFF, ATTENUATION_CUTOFF * power) / power;

		return 1.0f / glm::sqrt(attenuation);
	}

}