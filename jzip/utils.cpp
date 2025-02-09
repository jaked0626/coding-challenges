#include <unordered_map>

#include "utils.h"

template <typename K, typename V>
std::unordered_map<V, K> reverse_map(const std::unordered_map<K, V>& map)
{
    std::unordered_map<V, K> reversed_map {};
    for (auto& [k, v] : map)
    {
        reversed_map[v] = k;
    }

    return reversed_map;
}