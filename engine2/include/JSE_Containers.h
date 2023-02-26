#ifndef JSE_CONTAINERS_H
#define JSE_CONTAINERS_H

using JsString = std::string;
template<typename T>
using JsVector = std::vector<T>;
template<typename K, typename V>
using JsHashMap = std::unordered_map<K, V>;
template<typename T>
using JsSet = std::unordered_set<T>;
template<typename T, size_t cap>
using JsArray = std::array<T, cap>;
template<typename T>
using JsUniquePtr = std::unique_ptr<T>;
template<typename T>
using JsSharedPtr = std::shared_ptr<T>;

#endif // !JseCONTAINERS_H
