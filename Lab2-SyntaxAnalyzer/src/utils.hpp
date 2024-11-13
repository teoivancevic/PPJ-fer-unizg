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

//možeš ovo tretirat ko obićnu mapu, sam je malo brže

using std::map;
using std::set;
using std::vector;
using std::pair;
using std::queue;

//ovo bi vjv trebalo definirati lokalno u klasama, ali se koriste dovoljno često izvan klasa pa je ok
using Symbol = std::string;
using State = int;
using Word = vector<Symbol>;

static const Symbol GRAMMAR_NEW_BEGIN_STATE = "<<S'>>";
static const Symbol eps = "$";
static const Symbol end_sym = "$";

using std::cin;
using std::cout;
using std::cerr;
using std::endl;

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
    for (int i = (int) vec.size() - 1; i>=0; i--) 
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
        for (int i=0; i < (int) vec1.size(); i++) { 
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

[[maybe_unused]]
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

[[maybe_unused]]
static std::string consumeNextWord (std::string& str, char del = ' ') {
    int i;
    std::string word;
    for (i = 0; i < (int) str.size(); i++) {
        if (str[i] == del) {
            word = str.substr(0, i);
            str = str.c_str() + i + 1;
            return word;
        }
    }
    word = std::move(str);
    return word;
}

[[maybe_unused]]
static std::string readNextWord (const std::string& str, int at = 0, char del = ' ') {
    std::string word;
    for (int i = at; i < (int) str.size(); i++)
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

template<template <typename, typename...> class container, typename T, typename ...Args>
std::string concatToString_r (const container<T, Args...>& c, const std::string delim = " ") {
    std::string rez = "";
    for (int i = (int) c.size() - 1; i > -1; i--) {
        rez += c[i];
        if (i) rez += delim;
    }
    return rez;
}
