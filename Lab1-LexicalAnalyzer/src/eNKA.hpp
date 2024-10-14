#pragma once
#include"Regex.hpp"
#include<map>
#include<tuple>
#include<unordered_set>
#include<vector>
#include"Utils.hpp"

struct FiniteAutomata 
{ 
    struct State;

    using ID = uint32_t;
    using S = char;
    template<typename T>
    using Set = std::unordered_set<T>;

    static const S EPS = 0;

private:
    std::vector<State> states;
    State* start;
    mutable bool is_accept = false;
    mutable Set<ID> currentState;

public:
    FiniteAutomata() {}

    FiniteAutomata(const Regex& regex) {
        start = make_state();
        parseRegex(regex, start)->is_valid = true;
    }

    State* get(ID id) {
        return &states[id];
    }

    struct State {
    friend FiniteAutomata;
        
        bool is_valid = false;
        const ID id;

    private:
        std::map<S, std::pair<Set<ID>, bool>> next;
        Set<ID> e_neighborhood;
        bool evaluated = true;
        bool optimized = true;
        FiniteAutomata* parent;
        
    public: 
        State(FiniteAutomata* parent) 
            : parent(parent), id(parent->states.size()) {}
        
        State* add(S edge = EPS) {
            State* new_state = parent->make_state();
            this->link(new_state, edge);
            return new_state;
        }

        void link(State* other, S s = EPS) {
            optimized = false;
            if (s == EPS) {
                evaluated = false;
                e_neighborhood.insert(other->id);
            } else 
                next[s].first.insert(other->id);
        }

        const Set<ID>& get_eps_neighbors() {
            evaluate_eps_neighborhood();
            return e_neighborhood;
        }

        State* get(ID id) {
            return &(parent->states[id]);
        }

    private:
        void evaluate_eps_neighborhood() {
            if (evaluated) return;
            
            Set<ID> new_neighborhood = e_neighborhood;
            for (ID id : e_neighborhood)
                make_set_union(new_neighborhood, get(id)->get_eps_neighbors());
            e_neighborhood = std::move(new_neighborhood);
            
            if (this->id == parent->start->id)
                for (ID id : e_neighborhood) 
                    is_valid |= get(id)->is_valid;
            
            evaluated = true;
        }

        void remove_eps_transitions() {
            if (this->id == parent->start->id || optimized) return;

            if (!e_neighborhood.empty())
                evaluate_eps_neighborhood();

            for (auto pair : next) {
                Set<ID>& old_set = pair.second.first;
                Set<ID> next = old_set;
                S s = pair.first;
                for (ID id : old_set)
                    make_set_union(next, get(id)->get_eps_neighbors());
                old_set = std::move(next);
                for (ID id : old_set)
                    pair.second.second |= get(id)->is_valid;
            }

            e_neighborhood.clear();
            optimized = true;
        }
        
        const Set<ID>* consume(S s) const {
            return &(next.at(s).first);
        };
    };

    State* make_state() {
        states.emplace_back(this);
        return &states.back();
    }

    void link(State* s1, State* s2, const S& s = EPS) {
        s1->link(s2, s);
    }

    bool evaluate (const std::string& str) {
        currentState = start->get_eps_neighbors();
        currentState.insert(start->id);
        is_accept = start->is_valid; 
        for (S s : str) 
            currentState = update_states(s);
        return is_accept;
    }

private:
    Set<ID> update_states(S s) {
        Set<ID> new_state;
        is_accept = false;
        for (ID id : currentState) {
            is_accept |= get(id)->next[s].second; 
            auto* ptr = get(id)->consume(s);
            if (ptr) make_set_union(new_state, *ptr);
        }
        return new_state;
    }

    State* parseRegex (const Regex& regex, State* state) {
        State* return_state;
        
        switch (regex.type())
        {
        case Regex::HAS_SEPARATOR:
            return_state = make_state();
            for (const Regex& r : regex) 
                parseRegex(r, state->add())->link(return_state);
            break;
        case Regex::HAS_JOIN:
            return_state = state;
            for (const Regex& r : regex) 
                return_state = parseRegex(r, return_state);
            break;
        case Regex::ATOMIC:
            return_state = state->add(regex.exp[0]);
            break;
        }

        if (regex.has_kleen())
            return_state->link(state);
        
        return return_state;
    }
};