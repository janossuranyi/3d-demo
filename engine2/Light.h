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
		float r;
		float g;
		float b;
		float power;
		glm::vec3 GetColorWithPower() const
		{
			return glm::vec3(r * power, g * power, b * power);
		}
		glm::vec3 GetColor() const
		{
			return glm::vec3(r, g, b);
		}
	};

	struct lightOpts_t
	{
		lightColor_t color;
		// Attenuation factors [quadratic, linear, constans]
		float radius;
		float innerConeAngle;
		float outerConeAngle;
		float diffuseFactor;
		float specularFactor;
		float attnCutoff;
		float invAttnCutoff;
		void SetRadius(float r) { radius = r; }
		void SetAttnCutoff(float c) 
		{
			attnCutoff = c;
			invAttnCutoff = 1.0f / c;
		}
		lightOpts_t()
		{
			attnCutoff = 0.001f;
			invAttnCutoff = 1.0f / attnCutoff;
			color = { 1.f,1.f,1.f,20.0f };
			radius = 0.0f;
			diffuseFactor = 1.0f;
			specularFactor = 1.0f;
			outerConeAngle = 30.0f * M_PI / 180.0f;
			innerConeAngle = outerConeAngle;
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
		float GetBounds() const;
		lightOpts_t opts;

	private:
		eLightType type;
		Node3D* node;
		Material const* shader;
	};
}