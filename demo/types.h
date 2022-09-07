#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;

typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef glm::quat quat;

typedef glm::uint uint;
typedef glm::ivec2 ivec2;


struct Entity {
	virtual void updateParentChild() = 0;
};