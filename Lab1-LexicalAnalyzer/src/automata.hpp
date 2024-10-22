#pragma once
#include"Regex.hpp"
#include<map>
#include<tuple>
#include<unordered_set>
#include<vector>
#include<queue>
#include<iostream>

class NKA { 
public:
    std::string name;
    std::vector<std::string> commands;

    using ID = uint32_t;
    using sym = char;
    template<typename T>
    using Set = std::unordered_set<T>;

    static const sym EPS = 0;

private:
    struct State;
    std::vector<State> states;
    ID start, end;

    mutable bool is_accept = false;
    mutable Set<ID> currentState;

    struct State {
        std::map<sym, Set<ID>> next;
        Set<ID> e_neighborhood;
        bool kleen = false;
        bool evaluated;
        bool optimized;
        bool acceptable;
    };

public:
    NKA();

    NKA(const char* str);

    NKA(const std::string& str);

    NKA(const Regex& regex);

    NKA(const NKA& atm);
    NKA(NKA&& atm) noexcept;

    void operator= (const char* str);

    void operator= (const std::string& str);

    void operator= (const NKA& regex);
    void operator= (NKA&& regex) noexcept;

    void operator= (const Regex& regex);

    bool evaluate (const std::string& str);
private:
    State& get(ID id);

    const State& get(ID id) const;

    ID make_state();

    ID add(ID state, sym s = EPS);

    void link(ID s1, ID s2, sym s = EPS);

    const Set<ID>& get_eps_neighbors(ID state) const;
    
    Set<ID>& get_eps_neighbors(ID state);

    Set<ID> unionize (const Set<ID>& states);

    void remove_eps_transitions(ID state, sym s);
    
    Set<ID> consume(const Set<ID>& set, sym s);

    Set<ID> bad_consume(const Set<ID>& set, sym s);

    ID parseRegex (const Regex& regex, ID state);
};