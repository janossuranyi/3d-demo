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
			color[0] = (byte)(255.0f * glm::clamp(pX[0], 0.0f, 1.0f));
			color[1] = (byte)(255.0f * glm::clamp(pX[1], 0.0f, 1.0f));
			color[2] = (byte)(255.0f * glm::clamp(pX[2], 0.0f, 1.0f));
			color[3] = (byte)(255.0f * glm::clamp(pX[3], 0.0f, 1.0f));
		}
		void SetColor(float* pX)
		{
			color[0] = (byte)(255.0f * glm::clamp(pX[0], 0.0f, 1.0f));
			color[1] = (byte)(255.0f * glm::clamp(pX[1], 0.0f, 1.0f));
			color[2] = (byte)(255.0f * glm::clamp(pX[2], 0.0f, 1.0f));
			color[3] = (byte)(255.0f * glm::clamp(pX[3], 0.0f, 1.0f));
		}
		void SetColor(float fX, float fY, float fZ, float fW)
		{
			color[0] = (byte)(255.0f * glm::clamp(fX, 0.0f, 1.0f));
			color[1] = (byte)(255.0f * glm::clamp(fY, 0.0f, 1.0f));
			color[2] = (byte)(255.0f * glm::clamp(fZ, 0.0f, 1.0f));
			color[3] = (byte)(255.0f * glm::clamp(fW, 0.0f, 1.0f));
		}
		void SetTangent(float fX, float fY, float fZ, float fW)
		{
			tangent[0] = (byte)(255.0f * ((1.0f + glm::clamp(fX, -1.0f, 1.0f)) / 2.0f));
			tangent[1] = (byte)(255.0f * ((1.0f + glm::clamp(fY, -1.0f, 1.0f)) / 2.0f));
			tangent[2] = (byte)(255.0f * ((1.0f + glm::clamp(fZ, -1.0f, 1.0f)) / 2.0f));
			tangent[3] = (byte)(255.0f * ((1.0f + glm::clamp(fW, -1.0f, 1.0f)) / 2.0f));
		}
		void SetTangent(const glm::vec4& pX)
		{
			tangent[0] = (byte)(255.0f * ((1.0f + glm::clamp(pX[0], -1.0f, 1.0f)) / 2.0f));
			tangent[1] = (byte)(255.0f * ((1.0f + glm::clamp(pX[1], -1.0f, 1.0f)) / 2.0f));
			tangent[2] = (byte)(255.0f * ((1.0f + glm::clamp(pX[2], -1.0f, 1.0f)) / 2.0f));
			tangent[3] = (byte)(255.0f * ((1.0f + glm::clamp(pX[3], -1.0f, 1.0f)) / 2.0f));
		}
		void SetTangent(float* pX)
		{
			tangent[0] = (byte)(255.0f * ((1.0f + glm::clamp(pX[0], -1.0f, 1.0f)) / 2.0f));
			tangent[1] = (byte)(255.0f * ((1.0f + glm::clamp(pX[1], -1.0f, 1.0f)) / 2.0f));
			tangent[2] = (byte)(255.0f * ((1.0f + glm::clamp(pX[2], -1.0f, 1.0f)) / 2.0f));
			tangent[3] = (byte)(255.0f * ((1.0f + glm::clamp(pX[3], -1.0f, 1.0f)) / 2.0f));
		}
		void SetNormal(float* pX)
		{
			normal[0] = (byte)(255.0f * ((1.0f + glm::clamp(pX[0], -1.0f, 1.0f)) / 2.0f));
			normal[1] = (byte)(255.0f * ((1.0f + glm::clamp(pX[1], -1.0f, 1.0f)) / 2.0f));
			normal[2] = (byte)(255.0f * ((1.0f + glm::clamp(pX[2], -1.0f, 1.0f)) / 2.0f));
			normal[3] = 127;
		}
		void SetNormal(const glm::vec3& pX)
		{
			normal[0] = (byte)(255.0f * ((1.0f + glm::clamp(pX[0], -1.0f, 1.0f)) / 2.0f));
			normal[1] = (byte)(255.0f * ((1.0f + glm::clamp(pX[1], -1.0f, 1.0f)) / 2.0f));
			normal[2] = (byte)(255.0f * ((1.0f + glm::clamp(pX[2], -1.0f, 1.0f)) / 2.0f));
			normal[3] = 127;
		}
		void SetNormal(float fX, float fY, float fZ)
		{
			normal[0] = (byte)(255.0f * ((1.0f + glm::clamp(fX, -1.0f, 1.0f)) / 2.0f));
			normal[1] = (byte)(255.0f * ((1.0f + glm::clamp(fY, -1.0f, 1.0f)) / 2.0f));
			normal[2] = (byte)(255.0f * ((1.0f + glm::clamp(fZ, -1.0f, 1.0f)) / 2.0f));
			normal[3] = 127;
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
		void SetUV(float* pX)
		{
			uv[0] = pX[0];
			uv[1] = pX[1];
		}
		void SetPos(float* pX)
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