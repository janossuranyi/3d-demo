#pragma once

#include <string>
#include <GL/glew.h>
#include "./GpuTypes.h"
#include "./RenderProgs.h"
#include "./RenderCommon.h"
#include "./Logger.h"

namespace jsr {

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
		float range;
		void CalculateRange();

		lightOpts_t()
		{
			color = lightColor_t( glm::vec3{1.f,1.f,1.f}, 5.0f );
			diffuseFactor = 1.0f;
			specularFactor = 1.0f;
			outerConeAngle = glm::radians(50.0f);
			innerConeAngle = outerConeAngle;
			linearAttn = 0.0f;
			expAttn = 1.0f;
			range = 0.0f;
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
		Light& SetShader(eShaderProg shader);
		eShaderProg GetShader() const;
		Node3D* GetNode() const;
		eLightType GetType() const;
		void SetId(int id);
		void SetName(const std::string& name);
		const std::string& GetName() const;
		inline int GetId() const { return id; }

		lightOpts_t opts;

	private:
		int id;
		float range;
		std::string name;
		eLightType type;
		Node3D* node;
		eShaderProg shader;
	};
}