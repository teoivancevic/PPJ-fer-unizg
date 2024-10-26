#pragma once
#include<unordered_set>
#include<unordered_map>
#include<vector>
#include<cstdint>
#include<memory>
#include<string>
#include<stdexcept>

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

static std::string file_no_extension(const std::string& str) {
    return str.substr(0, str.find_first_of('.'));
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
    return str.substr(at, str.size()-at);
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

template <bool cond, typename A, typename B>
struct meta_elif {
    using type = A;
};

template <typename A, typename B>
struct meta_elif<false, A, B> {
    using type = B;
};

template <typename A, typename B>
struct unordered_pair : std::pair<A, B> {
    unordered_pair(A a, B b) : std::pair<A, B>(a, b) {};
    
    template <template <typename, typename> class PAIR>
    bool operator== (const PAIR<A, B>& other) {
        return this->first == other.first && this->second == other.second;
    }
    template <template <typename, typename> class PAIR>
    bool operator== (const PAIR<B, A>& other) {
        return this->second == other.first && this->first == other.second;
    }
};

template <typename A>
struct unordered_pair<A, A> : std::pair<A, A> {
    unordered_pair(A a, A b) : std::pair<A, A>(a, b) {};

    template <template <typename, typename> class PAIR>
    bool operator== (const PAIR<A, A>& other) {
        return this->first == other.first && this->second == other.second
            || this->second == other.first && this->first == other.second;
    }
};

template<typename T>
struct Initializer {
    T init;
    Initializer(T init) : init(std::move(init)) {}
    Initializer(Initializer<T>&& init) : init(std::move(init.init)) {}
    Initializer() {}
};

template<typename T>
constexpr bool predicate_true (T x) {
    return true;
}

template<typename T>
constexpr bool predicate_false (T x) {
    return false;
}

template<typename T>
void make_set_union(std::unordered_set<T>& s1, const std::unordered_set<T>& s2) {
    for (const T& el : s2) 
        s1.insert(el);
}

template<typename T>
std::unordered_set<T> set_union_of(const std::unordered_set<T>& s1, const std::unordered_set<T>& s2) {
    std::unordered_set<T> rez = s1;
    make_set_union(rez, s2);
    return rez;
}