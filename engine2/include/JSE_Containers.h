#ifndef JSE_CONTAINERS_H
#define JSE_CONTAINERS_H

using JSE_String = std::string;
template<typename T>
using JSE_Vector = std::vector<T>;
template<typename K, typename V>
using JSE_HashMap = std::unordered_map<K, V>;
template<typename T>
using JSE_Set = std::unordered_set<T>;
template<typename T, size_t cap>
using JSE_Array = std::array<T, cap>;
template<typename T>
using JSE_UniqePtr = std::unique_ptr<T>;
template<typename T>
using JSE_SharedPtr = std::shared_ptr<T>;

#endif // !JSE_CONTAINERS_H
