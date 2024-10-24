#pragma once
#include"Regex.hpp"
#include<map>
#include<tuple>
#include<unordered_set>
#include<vector>
#include<queue>
#include<iostream>
#include<algorithm>

/*
    Koristi push_symbol da dodaješ simbole u automat, funkcija vraća je li novo stanje prihvatljivo ili ne
    pop_symbol se vraća u prethodno stanje, reset se vraća u početno stanje
    eval provjerava niz znakova odjednom i vraća se u početno stanje pri završetku (vjv manje korisno)
    name i commands su javni članovi u kojima se nalaze informacije o pravilu koje automat predstavlja
*/

class NKA {
public:
    std::string name;
    std::vector<std::string> commands;

    using ID = uint32_t;
    using sym = char;
    template<typename T>
    using Set = std::unordered_set<T>;

    static const sym EPS;
    ID start, end;
    
private:
    struct State;
    std::vector<State> states;

    mutable bool is_accept = false;
    mutable std::vector<Set<ID>> stack;

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

    size_t size();

    const Set<ID>& currentState() const;

    bool eval (const std::string& str);

    bool push_sym (sym s);

    void pop_sym ();

    const State& get(ID id) const;

    ID make_state();

    ID add(ID state, sym s = EPS);

    void link(ID s1, ID s2, sym s = EPS);

    void reset();

    const Set<ID>& get_transitions(ID id, sym s = EPS);
    Set<sym> get_transition_symbols(ID id);
    
private:

    State& get(ID id);

    const Set<ID>& get_eps_neighbors(ID state) const;
    
    Set<ID>& get_eps_neighbors(ID state);

    Set<ID> unionize (const Set<ID>& states);

    void remove_eps_transitions(ID state, sym s);
    
    Set<ID> consume(const Set<ID>& set, sym s);

    ID parseRegex (const Regex& regex, ID state);
};