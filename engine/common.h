#pragma once

//#include <SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/quaternion.hpp>
#include <stb_image.h>

#include <condition_variable>
#include <cinttypes>
#include <chrono>
#include <vector>
#include <array>
#include <variant>
#include <thread>
#include <mutex>
#include <string>
#include <utility>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

// basic type aliases

using String = std::string;
template<typename T>
using Vector = std::vector<T>;
template<typename K, typename V>
using HashMap = std::unordered_map<K, V>;
template<typename T>
using Set = std::unordered_set<T>;
template<typename T, size_t size>
using Array = std::array<T, size>;
template<typename T>
using UniqePtr = std::unique_ptr<T>;
template<typename T>
using SharedPtr = std::shared_ptr<T>;
using Mutex = std::mutex;
using Thread = std::thread;
using ConditionVar = std::condition_variable;

using uint = unsigned int;
using int8 = int8_t;
using uint8 = uint8_t;
using int16 = int16_t;
using uint16 = uint16_t;
using ushort = uint16_t;
using int32 = int32_t;
using uint32 = uint32_t;
using int64 = int64_t;
using uint64 = uint64_t;
using ivec2 = glm::ivec2;
using ivec3 = glm::ivec3;
using ivec4 = glm::ivec4;
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat3 = glm::mat3;
using mat4 = glm::mat4;
using quat = glm::quat;
using uchar = unsigned char;

#ifndef M_PI
#define M_PI    3.14159265358979323846264338327950288   /**< pi */
#endif
