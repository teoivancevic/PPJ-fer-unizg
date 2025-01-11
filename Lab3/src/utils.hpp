#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <map>
#include <set>
#include <queue>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <stdexcept>

using namespace std;

// provjera za mape
template <typename T, typename K>
inline bool exists(const map<K, T> &m, const K &state)
{
    return (bool)m.count(state);
}
