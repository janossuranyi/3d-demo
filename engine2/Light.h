#pragma once

#include <string>
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
		lightColor_t(glm::vec3 color, float intensity) { this->color = { color.r,color.g,color.b,intensity }; }
		lightColor_t() : color(glm::vec4(1.0f)) {}
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
		float radius;
		void CalculateRadius()
		{
			const auto r = -linearAttn + glm::sqrt(linearAttn * linearAttn - 4.0f * expAttn * (1.0f - 256.0f * color.GetPower()));
			radius = r / (2.0f * expAttn);

			/*
		const float INTENSITY_CUTOFF = 1.0f;
		const float ATTENUATION_CUTOFF = 1.0f / 256.0f;
		auto power = opts.color.GetPower() / (4.0f * glm::pi<float>());
		auto attenuation = glm::max(INTENSITY_CUTOFF, ATTENUATION_CUTOFF * power) / power;

		radius = 1.0f / glm::sqrt(attenuation);

			*/
		}

		lightOpts_t()
		{
			color = lightColor_t( glm::vec3{1.f,1.f,1.f}, 5.0f );
			diffuseFactor = 1.0f;
			specularFactor = 1.0f;
			outerConeAngle = glm::radians(50.0f);
			innerConeAngle = outerConeAngle;
			linearAttn = 0.0f;
			expAttn = 1.0f;
			radius = 0.0f;
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
		void SetId(int id);
		void SetName(const std::string& name);
		const std::string& GetName() const;
		inline int GetId() const { return id; }

		lightOpts_t opts;

	private:
		int id;
		float radius;
		std::string name;
		eLightType type;
		Node3D* node;
		Material const* shader;
	};
}