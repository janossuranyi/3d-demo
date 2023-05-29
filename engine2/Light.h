#pragma once

#include <GL/glew.h>
#include "./GpuTypes.h"

namespace jsr {

	enum eLightType
	{
		LIGHT_DIRECTED,
		LIGHT_POINT,
		LIGHT_SPOT,
		LIGHT_BOX,
		LIGHT_COUNT
	};

	struct lightColor_t
	{
		glm::vec4 color;
		glm::vec3 GetColorWithPower() const
		{
			return glm::vec3(color) * color.w;
		}
		glm::vec3 GetColor() const
		{
			return glm::vec3(color);
		}
		float GetPower() const { return color.w; }
	};

	struct lightOpts_t
	{
		lightColor_t color;
		// Attenuation factors [quadratic, linear, constans]
		float linearAttn;
		float expAttn;
		float innerConeAngle;
		float outerConeAngle;
		float diffuseFactor;
		float specularFactor;
		lightOpts_t()
		{
			color = { {1.f,1.f,1.f,20.0f } };
			diffuseFactor = 1.0f;
			specularFactor = 1.0f;
			outerConeAngle = 50.0f;
			innerConeAngle = outerConeAngle;
			linearAttn = 0.0f;
			expAttn = 1.0f;
		}
	};

	class Material;
	class Node3D;
	class Light
	{
	public:
		Light();
		Light(eLightType type);
		Light(eLightType type, Node3D* pNode);
		Light(eLightType type, Node3D* pNode, Material const* pMaterial);
		Light& SetNode(Node3D* pNode);
		Light& SetMaterial(Material const* pMaterial);
		Node3D* GetNode() const;
		eLightType GetType() const;
		Material const* GetMaterial() const;
		float GetRadius() const;
		float GetRadius2() const;
		lightOpts_t opts;

	private:
		eLightType type;
		Node3D* node;
		Material const* shader;
	};
}