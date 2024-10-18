#pragma once
#include"Regex.hpp"
#include<map>
#include<tuple>
#include<unordered_set>
#include<vector>
#include<queue>
#include"Utils.hpp"

struct NKA 
{ 
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
        bool evaluated;
        bool optimized;
        bool acceptable;
    };

public:

    NKA(const Regex& regex) {
        start = make_state();
        end = parseRegex(regex, start);
    }

    bool evaluate (const std::string& str) {
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

private:
    State& get(ID id) {
        return states[id];
    }

    ID make_state() {
        states.emplace_back();
        return states.size()-1;
    }

    ID add(ID state, sym s = EPS) {
        ID next = make_state();
        link(state, next, s);
        return next;
    }

    void link(ID s1, ID s2, sym s = EPS) {
        if (s == EPS)
            get(s1).e_neighborhood.insert(s2);
        else 
            get(s1).next[s].insert(s2);
    }
    
    Set<ID>& get_eps_neighbors(ID state) {

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

    Set<ID> unionize (const Set<ID>& states) {
        Set<ID> neighborhood = states;

        for (ID id : states)
            make_set_union(neighborhood, get_eps_neighbors(id));

        return neighborhood;
    }

    void remove_eps_transitions(ID state, sym s) {
        if (get(state).optimized) 
            return;
        get(state).optimized = true;

        Set<ID>& eps = get_eps_neighbors(state);
        get(state).next[s] = bad_consume(eps, s);

        eps.clear();
    }    
    
    Set<ID> consume(const Set<ID>& set, sym s) {
        Set<ID> rez;

        for (ID state : set) {
            if (state != start) remove_eps_transitions(state, s);
            make_set_union(rez, get(state).next[s]);
        }

        return unionize(rez);
    }

    Set<ID> bad_consume(const Set<ID>& set, sym s) {
        Set<ID> rez;

        for (ID state : set)
            make_set_union(rez, get(state).next[s]);

        return unionize(rez);
    }

    ID parseRegex (const Regex& regex, ID state) {
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
            link(return_state, state);
            return_state = state;
        }
        
        return return_state;
    }
};
/*
class DKA 
{ 
    struct State;

    using ID = uint32_t;
    using sym = char;
    
    std::queue<ID> slots;
    std::vector<State> states;

    ID global_id() {
        return states.size() + 1;
    }
    ID available() {
        if (slots.empty())
            return global_id();
        ID id = slots.front();
        slots.pop();
        return id;
    }

    ID start, end;

    mutable ID currentState;

public:
    DKA(const Regex& regex) {
        auto t = construct_from_regex(regex);
        start = t.first;
        end = t.second;
    }

    bool evaluate(std::string exp) {
        return false;
    }

private:
    std::pair<ID, ID> construct_from_regex (const Regex& regex) {
        ID new_start = make_state(), new_end;
        
        if (regex.type() == Regex::HAS_SEPARATOR) {
            new_end = make_state();
            bool kleened = false;
            for (const Regex& r : regex) {
                auto t = construct_from_regex(r);
                ID end_s = t.second, start_s = t.first;
                fuse(new_start, start_s);
                if (t.second = t.first && !kleened) {
                    kleened = true;
                    fuse(new_end, new_start);
                    new_start = new_end;
                } else 
                    fuse(new_end, end_s);
            }
        } else if (regex.type() == Regex::HAS_JOIN) {
            ID start_s = new_start;
            for (const Regex& r : regex) {
                auto t = construct_from_regex(r);
                get(start_s).acceptable = false;
                fuse(start_s, t.first);
                start_s = t.first == t.second ? start_s : t.second;
            }
            new_end = start_s;
        } else
            new_end = add(new_start, regex.exp[0]);

        if (regex.has_kleen()) {
            fuse(new_start, new_end);
            new_end = new_start;
        }

        get(new_end).acceptable = true;
        return {new_start, new_end};
    }

    State& get(ID id) {
        return states[id-1];
    }

    ID make_state() {
        ID id = available();
        if (id > states.size())
            states.emplace_back();
        return id;
    }

    struct State {
        std::map<sym, ID> next;
        std::map<sym, ID> watchers;
        bool dead;
        bool acceptable = false;

        State() {}

        State(const State& m) {
            operator=(m);
        }
        
        State(State&& m) noexcept {
            operator=(m);
        }

        void operator=(const State& m) { 
            this->watchers = m.watchers;
            this->next = m.next;
            printf("copied\n");
        }

        void operator=(State&& m) noexcept { 
            this->watchers = std::move(m.watchers);
            this->next = std::move(m.next);
            printf("moved\n");
        }
    };
    
    ID add(ID state, sym edge) {
        ID new_state = make_state();
        link(state, new_state, edge);
        return new_state;
    }

    void link(ID first, ID second, sym s) {
        get(first).next[s] = second;
        get(second).watchers[s] = first;
    }

    void fuse(ID usurper, ID other) {
        for (auto pair : get(other).next) {
            ID new_id = pair.second;
            sym s = pair.first;
            if (get(usurper).next[s])
                fuse(get(usurper).next[s], new_id);
            else {
                get(usurper).next[s] = new_id;
                get(new_id).watchers[s] = usurper;
            }
        }

        for (auto pair : get(other).watchers) {
            ID w_id = pair.second;
            sym s = pair.first;
            get(w_id).next[s] = usurper;
            get(usurper).watchers[s] = w_id;
        }

        get(other).dead = true;
        slots.push(other);
        get(other).watchers.clear();
        get(other).next.clear();
    }
    
    ID consume(ID consumer, sym s) {
        return get(consumer).next[s];
    }
};

*/