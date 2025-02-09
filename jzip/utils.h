#pragma once

#include <unordered_map>

template <typename K, typename V>
std::unordered_map<V, K> reverse_map(const std::unordered_map<K, V>& map);