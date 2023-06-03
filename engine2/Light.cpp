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
		shader(PRG_DEFERRED_LIGHT),
		range(0.0f){}

	Light& Light::SetNode(Node3D* pNode)
	{
		node = pNode;
		return *this;
	}

	Light& Light::SetShader(eShaderProg shader)
	{
		this->shader = shader;
		return *this;
	}

	eShaderProg Light::GetShader() const
	{
		return shader;
	}

	Node3D* Light::GetNode() const
	{
		return node;
	}

	eLightType Light::GetType() const
	{
		return type;
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

	void lightOpts_t::CalculateRange()
	{
#if 0
		float x = linearAttn * linearAttn - 4.0f * expAttn * (1.0f - 32.0f * color.GetPower());
		float r = -linearAttn + glm::sqrt(x);
		range = r / (2.0f * expAttn);

#else
		constexpr float INTENSITY_CUTOFF = 0.005f;
		constexpr float ATTENUATION_CUTOFF = 1.0f / 255.0f;
		auto power = color.GetPower() / (4.0f * glm::pi<float>());
		auto attenuation = glm::max(INTENSITY_CUTOFF, ATTENUATION_CUTOFF * power) / power;

		range = 1.0f / glm::sqrt(attenuation);
#endif
		Info("int: %f, radius: %f", color.GetPower(), range);
	}	

}