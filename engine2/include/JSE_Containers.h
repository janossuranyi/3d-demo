#ifndef JSE_CONTAINERS_H
#define JSE_CONTAINERS_H

using JseString = std::string;
template<typename T>
using JseVector = std::vector<T>;
template<typename K, typename V>
using JseHashMap = std::unordered_map<K, V>;
template<typename T>
using JseSet = std::unordered_set<T>;
template<typename T, size_t cap>
using JseArray = std::array<T, cap>;
template<typename T>
using JseUniqePtr = std::unique_ptr<T>;
template<typename T>
using JseSharedPtr = std::shared_ptr<T>;

#endif // !JseCONTAINERS_H
