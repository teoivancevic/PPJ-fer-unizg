#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <set>
#include <queue>

//možeš ovo tretirat ko obićnu mapu, sam je malo brže
template <typename K, typename V, typename Hash = std::hash<K>, typename Compare = std::equal_to<K>>
using map = std::unordered_map<K, V, Hash, Compare>;

//ovo su stvari koje često kostistimo pa je ok uvest ih iz namespacea, 
//ali string namjerno nisam stavio da se sjetimo koristi aliase
using std::set;
using std::vector;
using std::pair;
using std::queue;

using std::cin;
using std::cout;
using std::cerr;

template <template <typename, typename...> class container, typename T, typename ...Args>
struct std::hash<container<T, Args...>> {
    std::size_t operator() (const set<T>& s) const {
        std::size_t hash = 0;
        for (const T& elem : s) {
            hash ^= std::hash<T>()(elem) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

template <template <typename, typename...> class container, typename T, typename ...Args>
struct std::equal_to<container<T, Args...>> {
    std::size_t operator() (const container<T>& s1, const container<T>& s2) const {
        if (s1.size() != s2.size()) return false;
        bool equal = true;
        for (const T& elem : s1) {
            equal &= s1.count(elem) == s2.count(elem);
        }
        return equal;
    }
};

template <typename T>
struct std::equal_to<vector<T>> {
    std::size_t operator() (const vector<T>& vec1, const vector<T>& vec2) const {
        if (vec1.size() != vec2.size()) return false;
        bool equal = true;
        for (int i=0; i<vec1.size(); i++) { 
            equal &= vec1[i] == vec2[i];
        }
        return equal;
    }
};

template <typename T>
struct std::hash<vector<T>> {
    std::size_t operator() (std::vector<T> const& vec) const {
        std::size_t seed = vec.size();
        for(const T& i : vec) {
            seed ^= std::hash<T>()(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

template<typename T>
using SetMap = map<set<T>, T>;