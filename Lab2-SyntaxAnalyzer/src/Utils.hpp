#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <map>
#include <set>
#include <queue>
#include<cstdint>
#include<memory>
#include<stdexcept>
// #include<deque>

/*vezano za using namespace std:
    radije ne bi to koristio jer je zbilja teško razlučit što je STL a što 
    naš kod. Umjesto tog koristi aliase kao ispod. 
*/

//možeš ovo tretirat ko obićnu mapu, sam je malo brže
// template <typename K, typename V, typename Hash = std::hash<K>, typename Compare = std::equal_to<K>>
// using map = std::unordered_map<K, V, Hash, Compare>;
using std::map;

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
    for (int i = vec.size() - 1; i>=0; i--) 
        rez.emplace_back(vec[i]);
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

static int to_int (const std::string& str) {
    int rez = 0;
    for (char digit : str)
        rez = rez * 10 + (digit - '0');
    return rez;
}

template<typename ... Args>
std::string string_format( const std::string& format, Args ... args )
{
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

static std::string consumeNextWord (std::string& str, char del = ' ') {
    int i;
    std::string word;
    for (i = 0; i<str.size(); i++) {
        if (str[i] == del) {
            word = str.substr(0, i);
            str = str.c_str() + i + 1;
            return word;
        }
    }
    word = std::move(str);
    return word;
}

static std::string readNextWord (const std::string& str, int at = 0, char del = ' ') {
    std::string word;
    for (int i = at; i<str.size(); i++)
        if (str[i] == del)
            return str.substr(at, i);
    return str;
}

template<typename ACT>
void forEachWord (std::string str, ACT action, char del = ' ') {
    while (!str.empty()) {
        std::string word = consumeNextWord(str, del);
        if (!word.empty())
            action(std::move(word));
    }
}

template<typename ACT>
void consumeEachWord (std::string& str, ACT action, char del = ' ') {
    while (!str.empty()) {
        std::string word = consumeNextWord(str, del);
        if (!word.empty())
            action(std::move(word));
    }
}

//provjera za mape
template<typename T, typename K>
inline bool exists(const map<K, T>& m, const K& state) {
    return (bool) m.count(state);
}