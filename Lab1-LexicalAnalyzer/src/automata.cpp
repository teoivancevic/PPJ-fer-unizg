#pragma once
#include"automata.hpp"

NKA::NKA() {}

NKA::NKA(const char* str) : NKA((Regex)str) {}

NKA::NKA(const std::string& str) : NKA((Regex)str) {}

NKA::NKA(const Regex& regex) {
    *this = regex;
}

NKA::NKA(const NKA& atm) {
    *this = atm;
}
NKA::NKA(NKA&& atm) noexcept {
    *this = std::move(atm);
}

void NKA::operator= (const char* str) {
    *this = Regex(str);
}

void NKA::operator= (const std::string& str) {
    *this = Regex(str);
}

void NKA::operator= (const NKA& regex) {
    states = regex.states;
    start = regex.start, end = regex.end;
}
void NKA::operator= (NKA&& regex) noexcept {
    states = std::move(regex.states);
    start = regex.start, end = regex.end;
}

void NKA::operator= (const Regex& regex) {
    states.clear();
    start = make_state();
    end = parseRegex(regex, start);
}

bool NKA::evaluate (const std::string& str) {
    currentState.clear();
    currentState = get_eps_neighbors(start);
    for (sym s : str) {
        if (currentState.empty())
            return false;
        currentState = bad_consume(currentState, s);
    }
    for (ID state : currentState)
        if (get(state).acceptable)
            return true;
    return false;
}

NKA::State& NKA::get(ID id) {
    return states[id];
}

const NKA::State& NKA::get(ID id) const {
    return states[id];
}

NKA::ID NKA::make_state() {
    states.emplace_back();
    return states.size()-1;
}

NKA::ID NKA::add(ID state, sym s) {
    ID next = make_state();
    link(state, next, s);
    return next;
}

void NKA::link(ID s1, ID s2, sym s) {
    if (s == EPS)
        get(s1).e_neighborhood.insert(s2);
    else 
        get(s1).next[s].insert(s2);
}

const NKA::Set<NKA::ID>& NKA::get_eps_neighbors(ID state) const {
    return get(state).e_neighborhood;
} 

NKA::Set<NKA::ID>& NKA::get_eps_neighbors(ID state) {

    Set<ID>& neighborhood = get(state).e_neighborhood;

    if (!get(state).evaluated) {
        get(state).evaluated = true;
        neighborhood = unionize(neighborhood);
        if (neighborhood.count(end) || state == end)
            get(state).acceptable = true;
    }

    neighborhood.insert(state);

    return neighborhood;
}

NKA::Set<NKA::ID> NKA::unionize (const Set<ID>& states) {
    Set<ID> neighborhood = states;

    for (ID id : states)
        make_set_union(neighborhood, get_eps_neighbors(id));

    return neighborhood;
}

void NKA::remove_eps_transitions(ID state, sym s) {
    if (get(state).optimized) 
        return;
    get(state).optimized = true;

    Set<ID>& eps = get_eps_neighbors(state);
    get(state).next[s] = bad_consume(eps, s);

    eps.clear();
}    

NKA::Set<NKA::ID> NKA::consume(const Set<ID>& set, sym s) {
    Set<ID> rez;

    for (ID state : set) {
        if (state != start) remove_eps_transitions(state, s);
        make_set_union(rez, get(state).next[s]);
    }

    return unionize(rez);
}

NKA::Set<NKA::ID> NKA::bad_consume(const Set<ID>& set, sym s) {
    Set<ID> rez;

    for (ID state : set)
        make_set_union(rez, get(state).next[s]);

    return unionize(rez);
}

NKA::ID NKA::parseRegex (const Regex& regex, ID state) {
    ID return_state;
    
    switch (regex.type())
    {
    case Regex::HAS_SEPARATOR:
        return_state = make_state();
        for (const Regex& r : regex) 
            link(parseRegex(r, add(state)), return_state);
        break;
    case Regex::HAS_JOIN:
        return_state = state;
        for (const Regex& r : regex) 
            return_state = parseRegex(r, add(return_state));
        break;
    case Regex::ATOMIC:
        return_state = add(state, regex.get());
        break;
    }

    if (regex.has_kleen()) {
        get(return_state).kleen = true;
        link(return_state, state);
        return_state = state;
    }
    
    return return_state;
}