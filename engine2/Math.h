#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

inline float GetMaxLightDistance(float intensity, float kl, float kq)
{
    return (-kl + glm::sqrt(kl * kl - 4.0f * kq * (1.0f - 256.0f * intensity))) / (2.0f * kq);
}
