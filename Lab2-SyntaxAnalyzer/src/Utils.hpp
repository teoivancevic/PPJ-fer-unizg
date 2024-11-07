#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <queue>

template <typename K, typename V, typename Hash = std::hash<K>, typename Compare = std::equal_to<K>>
using map = std::unordered_map<K, V, Hash, Compare>;

template <typename T>
using set = std::unordered_set<T>;

using std::string;
using std::vector;
using std::pair;
using std::queue;
using std::cin;
using std::cout;
using std::cerr;

using State = string;
using Symbol = string;

template<typename T>
struct setHash {
    std::size_t operator() (const set<T>& s) const {
        std::size_t hash = 0;
        for (const auto& elem : s)
            hash ^= std::hash<T>{}(elem) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        return hash;
    }
};

template<typename T>
struct setEqual {
    std::size_t operator() (const set<T>& s1, const set<T>& s2) const {
        bool equal = true;
        for (const auto& elem : s1)
            equal &= s2.count(elem);
        return equal;
    }
};

template<typename T>
using SetMap = map<set<T>, T, setHash<T>, setEqual<T>>;

template <typename T>
constexpr setHash hash_t = setHash<T>{};