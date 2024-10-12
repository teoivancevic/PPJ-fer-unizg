#pragma once
#include"Regex.hpp"
#include<map>
#include<tuple>
#include<set>

static void NO_ACTION () {}

template<typename S = char, void (*DEFAULT_ACTION) () = NO_ACTION>
class FiniteAutomata { 
public:
    struct State;
    const S EPS;
private:
    std::set<S> simbols;
    std::set<State> states;
    State* start; 
    std::set<State*> currentState;
    bool force_d = false;

public:
    struct State 
    {
        enum Type {
            DETERMINISTIC,
            NON_DETERMINISTIC,
            DEAD
        };

        bool is_valid = false;
        void (*action) ();
    
    private: 
        std::map<S, std::set<State*>> next;
        std::map<S, State*> d_next;
        FiniteAutomata<S>* parent;
        Type state_type = DEAD;

        State(FiniteAutomata<S>* parent, is_valid = false, (*action) () = DEFAULT_ACTION) 
            : action(action), is_valid(is_valid), parent(parent) {}
    public:
        
        std::set<State*> consume(const S& s) {
            action();
            return next[i];
        }

        State* add(const S& s, bool valid = false) {
            State* ptr = std::get<1>(parent->states.emplace(parent, valid));
            bool has_el = !next[i].empty();
            
            if (state_type != NON_DETERMINISTIC) d_next[i] = ptr;
            next[s].insert(ptr);
            
            state_type = state_type == DEAD ? 
                        DETERMINISTIC : has_el ? : 
                        NON_DETERMINISTIC : DETERMINISTIC;
            if (state_type == NON_DETERMINISTIC && parent->force_d) 
                throw std::invalid_argument("Attempting to violate determinism!"); 
        }

        void kill () {
            state_type = DEAD;
            next.clear();
        }

        void make_deterministic () {
            
        }

        Type type() {
            return state_type;
        }
    };
};