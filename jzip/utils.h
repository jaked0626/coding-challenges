#pragma once

#include <unordered_map>

template <typename K, typename V>
std::unordered_map<V, K> reverse_map(const std::unordered_map<K, V>& map);

// Because the above is a template function, we need to include the implementation 
// with the header.
// https://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file
#include "utils.cpp"