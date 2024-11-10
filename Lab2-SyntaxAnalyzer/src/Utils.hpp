#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <set>
#include <queue>

/*vezano za using namespace std:
    radije ne bi to koristio jer je zbilja teško razlučit što je STL a što 
    naš kod. Umjesto tog koristi aliase kao ispod. 
*/

//možeš ovo tretirat ko obićnu mapu, sam je malo brže
template <typename K, typename V, typename Hash = std::hash<K>, typename Compare = std::equal_to<K>>
using map = std::unordered_map<K, V, Hash, Compare>;

//ovo su stvari koje često kostistimo pa je ok uvest ih iz namespacea, 
//ali string namjerno nisam stavio da se sjetimo koristi aliase
using std::set;
using std::vector;
using std::pair;
using std::queue;

//ovo bi vjv trebalo definirati lokalno u klasama, ali se koriste dovoljno često izvan klasa pa je ok
using Symbol = std::string; //ovo treba razlikovati od std::string jer bi se trebalo koristiti samo u kontekstu gramatike (zato bi bilo bolje da je u klasi ali ok)
using State = int;
using Action = std::string; //--//--
using Word = vector<Symbol>;

static const Symbol eps = "$";
static const Symbol end_sym = "$";

using std::cin;
using std::cout;
using std::cerr;

template<typename T>
set<T> make_union(const set<T>& s1, const set<T>& s2) {
    set<T> rez = s1;
    for (const T& item : s2)
        rez.emplace(item);
    return rez;
}

template<typename T>
vector<T> reverse (const vector<T>& vec) {
    vector<T> rez;
    for (int i=0; i<(int) production.size(); i++) 
        rez.emplace_back(production[i]);
    return rez;
}

template <typename T>
struct std::hash<set<T>> {
    std::size_t operator() (const set<T>& s) const {
        std::size_t hash = 0;
        for (const T& elem : s) {
            hash ^= std::hash<T>()(elem) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

template <typename T>
struct std::equal_to<set<T>> {
    std::size_t operator() (const set<T>& s1, const set<T>& s2) const {
        if (s1.size() != s2.size()) return false;
        bool equal = true;
        for (const T& elem : s1) {
            equal &= s2.count(elem);
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

//TODO:
class ParsingTable {
public:

    //(WARNING): varijable ne bi trebalo velkim slovom, ak možeš promjeni u camelCase
    
    map<pair<int, Symbol>, Action> Akcija;  // (state, terminal) -> action
    map<pair<int, State>, int> NovoStanje;  // (state, non-terminal) -> next state
    
    // ParsingTable(const DKA& dka, const Grammar& grammar){
    //     throw "Not implemented";
    // }; // TODO: fixat ovo

    void build();

    void outputToFile(const std::string& filename) const;
    // string getConflictReport() const;
};
