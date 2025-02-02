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

template <typename T, typename K>
inline bool exists(const map<K, T> &m, const K &state)
{
    return (bool)m.count(state);
}

[[maybe_unused]]
static std::string consumeNextWord(std::string &str, char del = ' ')
{
    int i;
    std::string word;
    for (i = 0; i < (int)str.size(); i++)
    {
        if (str[i] == del)
        {
            word = str.substr(0, i);
            str = str.c_str() + i + 1;
            return word;
        }
    }
    word = std::move(str);
    return word;
}

[[maybe_unused]]
static std::string readNextWord(const std::string &str, int at = 0, char del = ' ')
{
    std::string word;
    for (int i = at; i < (int)str.size(); i++)
        if (str[i] == del)
            return str.substr(at, i);
    return str;
}

template <typename ACT>
void forEachWord(std::string str, ACT action, char del = ' ')
{
    while (!str.empty())
    {
        std::string word = consumeNextWord(str, del);
        if (!word.empty())
            action(std::move(word));
    }
}

template <typename ACT>
void consumeEachWord(std::string &str, ACT action, char del = ' ')
{
    while (!str.empty())
    {
        std::string word = consumeNextWord(str, del);
        if (!word.empty())
            action(std::move(word));
    }
}
