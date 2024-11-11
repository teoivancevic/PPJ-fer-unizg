#pragma once

#include "Utils.hpp"
#include "Grammar.hpp"

//Kužim što si radio s LRItem, ali da pojednostavimo stvari, lakše je mapirati ih u brojeve i onda koristiti njih kao State

//PROVJERI:
class eNKA 
{
    State ID = 1;

public:

    template<typename T>
    using StateMap = map<State, T>;
        //ovo bi mogo biti vektor, ali da izbjegnemo spammanje push_back() i 
        //potencijalan illegal mem access ovo je safer, plus lakše je adaptirat ako State nije int

//READ ONLY, public za easy access
    mutable map<LR1Item, State> states; //pretvorba LR1Item -> State
    mutable StateMap<LR1Item> items;
    mutable StateMap<map<Symbol, set<State>>> transitions;
    mutable set<Symbol> symbols = {end_sym};
    const State q0 = 0;

public:
    eNKA() {} //kada je definiran drugi konstruktor default se treba definirati eksplicitno

    eNKA(const Grammar& grammar)
    {
        const Symbol& S = grammar.BEGIN_SYMBOL;
        symbols.emplace(S);
        transitions.at(q0)[eps].insert(
            newState(
                {S, grammar.PRODUKCIJE.at(S).at(0), {end_sym}}
            )
        );

        for(const auto& [leftSymbol, productions] : grammar.PRODUKCIJE) // prodji kroz sve produkcije
        for(const Word& production : productions)
        { 
            LR1Item item (leftSymbol, production);
            State state = getState(item);

            while (!item.isComplete())
            {
                const Symbol& nextSym = item.symbolAfterDot();
                symbols.emplace(nextSym);

                transitions[state][eps] = computeEpsilonTransitions(state, grammar);
                transitions[state][nextSym].insert(state = getState(item.shift_dot_r()));
            }      
        }
    }

    //eps okolina od startState
    inline set<State>& start() const {
        return transitions[q0][eps];
    }

    //ovo radi tek nakon sto se svi prijelazi izracunaju
    set<State> get_next(const set<State>& current, const Symbol& sym) const 
    {
        set<State> result;

        for (State state : current) 
            for (State next : transitions.at(state).at(sym))
                result.emplace(next);

        return result;
    }

    const set<State>& get_epsilon(State state, const Grammar& grammar)
    {
        if (!transitions.at(state).count(eps))
            transitions.at(state)[eps] = computeEpsilonTransitions(state, grammar);
        
        return transitions.at(state).at(eps);
    }

    inline std::size_t size() {
        return ID;
    }

private:
    set<State> computeEpsilonTransitions(State state, const Grammar& grammar)
    {
        set<State> result;
        
        queue<LR1Item> q;
        q.push(items[state]);
        
        while (!q.empty()) 
        {
            LR1Item current = q.front();
            State state = getState(current);
            q.pop();

            result.insert(state);
            
            if (current.isComplete()) 
                continue;
            
            Symbol nextSymbol = current.symbolAfterDot();
            current.shift_dot_r();
            
            if (grammar.PRODUKCIJE.count(nextSymbol))
            { 
                for (const Word& production : grammar.PRODUKCIJE.at(nextSymbol)) 
                {
                    const Word& afterWord = current.after_dot;
                    LR1Item next_item (
                        nextSymbol, production, 
                        make_union(
                            grammar.startsWith(afterWord), 
                            (grammar.isVanishing(afterWord) ? 
                                current.lookahead : set<Symbol>{}
                            )
                        )
                    );
                    if (!result.count(getState(next_item))) 
                        q.push(next_item); 
                }
            }
        }

        return result;
    }

    //ovo se generalno ne bi trebalo koristit direktno kao ni states
    State newState(const LR1Item& item) {
        if (states.count(item)) {
            // printf("duplicate state: %s\n", item.toString().c_str()); //ne bi se trebalo dogoditi i think
            return -1;
        }
        items[ID] = item;
        return states[item] = ID++;
    }

    //ovo ili vrati postojeci state il ga na pravi pa vrati
    State getState(const LR1Item& item) {
        if (states.count(item))
            return states[item];
        return newState(item);
    }
};

class DKA 
{
    State ID = 0;

public:
    template<typename T>
    using StateMap = map<State, T>;

// private:
    StateMap<set<LR1Item>> items;
    StateMap<map<Symbol, State>> transitions;
    State start;
    mutable State currentState;

public:
    DKA() {}

    DKA(const eNKA& enka) 
    {
        SetMap<State> mapper;
        queue<set<State>> state_queue;
        state_queue.push(enka.start());

        mapper[enka.start()] = ID++;

        while (!state_queue.empty()) 
        {
            set<State> current = std::move(state_queue.front());
            State id = mapper[current];    
            state_queue.pop();

            for (State state : current)
                items[id].insert(enka.items.at(state));
                
            for (const Symbol& sym : enka.symbols) 
            {
                set<State> next = enka.get_next(current, sym);
                if (!mapper.count(next)) 
                {
                    transitions[id][sym] = ID;
                    mapper[next] = ID++;
                    state_queue.emplace(std::move(next));
                }
            }
        }
    }   

    inline void reset() const {
        currentState = start;
    }

    inline void update(const Symbol& sym) const {
        currentState = transitions.at(currentState).at(sym);
    }

    inline const set<LR1Item>& items() const {
        return items.at(currentState);
    } 

    inline std::size_t size() const {
        return ID;
    }
};