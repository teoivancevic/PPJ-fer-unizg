#include<unordered_set>
#include<unordered_map>
#include<vector>
#include<cstdint>

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