#pragma once

#include <glm/glm.hpp>
#include "./GpuTypes.h"
#include "./EngineTypes.h"

namespace jsr {
	struct drawVert_t {
		glm::vec3	xyz;
		glm::vec2	uv;
		byte		normal[4];
		byte		tangent[4];
		byte		color[4];

		glm::vec3 GetPos() const
		{
			return xyz;
		}
		glm::vec2 GetUV() const
		{
			return uv;
		}
		glm::vec4 GetTangent() const
		{			
			return glm::vec4(
				VERTEX_BYTE_TO_FLOAT(tangent[0]),
				VERTEX_BYTE_TO_FLOAT(tangent[1]),
				VERTEX_BYTE_TO_FLOAT(tangent[2]),
				VERTEX_BYTE_TO_FLOAT(tangent[3]));
		}
		glm::vec4 GetNormal() const
		{
			return glm::vec4(
				VERTEX_BYTE_TO_FLOAT(normal[0]),
				VERTEX_BYTE_TO_FLOAT(normal[1]),
				VERTEX_BYTE_TO_FLOAT(normal[2]),
				VERTEX_BYTE_TO_FLOAT(normal[3]));
		}
		glm::vec4 GetColor() const
		{
			return glm::vec4(
				unorm8ToFloat(color[0]),
				unorm8ToFloat(color[1]),
				unorm8ToFloat(color[2]),
				unorm8ToFloat(color[3]));
		}
		void SetColor(const glm::vec4& pX)
		{
			for (int i = 0; i < 4; ++i)
			{
				color[i] = floatToUnorm8(pX[i]);
			}
		}
		void SetColor(float const* pX)
		{
			for (int i = 0; i < 4; ++i)
			{
				color[i] = floatToUnorm8(pX[i]);
			}
		}
		void SetColor(float fX, float fY, float fZ, float fW)
		{
			color[0] = floatToUnorm8(glm::clamp(fX, 0.0f, 1.0f));
			color[1] = floatToUnorm8(glm::clamp(fY, 0.0f, 1.0f));
			color[2] = floatToUnorm8(glm::clamp(fZ, 0.0f, 1.0f));
			color[3] = floatToUnorm8(glm::clamp(fW, 0.0f, 1.0f));
		}
		void SetTangent(float fX, float fY, float fZ, float fW)
		{
			tangent[0] = VERTEX_FLOAT_TO_BYTE(saturate(fX));
			tangent[1] = VERTEX_FLOAT_TO_BYTE(saturate(fY));
			tangent[2] = VERTEX_FLOAT_TO_BYTE(saturate(fZ));
			tangent[3] = VERTEX_FLOAT_TO_BYTE(saturate(fW));
		}
		void SetTangent(const glm::vec4& pX)
		{
			for (int i = 0; i < 4; ++i)
			{
				tangent[i] = VERTEX_FLOAT_TO_BYTE(saturate(pX[i]));
			}
		}
		void SetTangent(float const* pX)
		{
			for (int i = 0; i < 4; ++i)
			{
				tangent[i] = VERTEX_FLOAT_TO_BYTE(saturate(pX[i]));
			}
		}
		void SetNormal(float const* pX)
		{
			for (int i = 0; i < 3; ++i)
			{
				normal[i] = VERTEX_FLOAT_TO_BYTE(saturate(pX[i]));
			}
			normal[3] = VERTEX_FLOAT_TO_BYTE(0.0f);
		}
		void SetNormal(const glm::vec3& pX)
		{
			for (int i = 0; i < 3; ++i)
			{
				normal[i] = VERTEX_FLOAT_TO_BYTE(saturate(pX[i]));
			}
			normal[3] = VERTEX_FLOAT_TO_BYTE(0.0f);
		}
		void SetNormal(float fX, float fY, float fZ)
		{
			normal[0] = VERTEX_FLOAT_TO_BYTE(saturate(fX));
			normal[1] = VERTEX_FLOAT_TO_BYTE(saturate(fY));
			normal[2] = VERTEX_FLOAT_TO_BYTE(saturate(fZ));
			normal[3] = VERTEX_FLOAT_TO_BYTE(0.0f);

		}
		void SetUV(const glm::vec2& v)
		{
			uv[0] = v.x;
			uv[1] = v.y;
		}
		void SetUV(float fX, float fY)
		{
			uv[0] = fX;
			uv[1] = fY;
		}
		void SetUV(float const* pX)
		{
			uv[0] = pX[0];
			uv[1] = pX[1];
		}
		void SetPos(float const* pX)
		{
			xyz[0] = pX[0];
			xyz[1] = pX[1];
			xyz[2] = pX[2];
		}
		void SetPos(float fX, float fY, float fZ)
		{
			xyz[0] = fX;
			xyz[1] = fY;
			xyz[2] = fZ;
		}
		void SetPos(const glm::vec2& v)
		{
			xyz[0] = v.x;
			xyz[1] = v.y;
			xyz[2] = 0.0f;
		}
		void SetPos(const glm::vec3& v)
		{
			xyz[0] = v.x;
			xyz[1] = v.y;
			xyz[2] = v.z;
		}
		void SetPos(const glm::vec4& v)
		{
			xyz[0] = v.x;
			xyz[1] = v.y;
			xyz[2] = v.z;
		}
	};

}