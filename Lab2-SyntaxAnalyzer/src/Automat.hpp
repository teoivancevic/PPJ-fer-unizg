#pragma once

#include "Utils.hpp"
#include "Grammar.hpp"

//Kužim što si radio s LRItem, ali da pojednostavimo stvari, lakše je mapirati ih u brojeve i onda koristiti njih kao State

//PROVJERI:
class eNKA 
{
    State ID = 0;

public:

    template<typename T>
    using StateMap = map<State, T>;
        //ovo bi mogo biti vektor, ali da izbjegnemo spammanje push_back() i 
        //potencijalan illegal mem access ovo je safer, plus lakše je adaptirat ako State nije int

//READ ONLY, public za easy access
    mutable map<LR1Item, State> states; //pretvorba LR1Item -> State
    mutable StateMap<LR1Item> items;
    mutable StateMap<map<Symbol, set<State>>> transitions;
    mutable set<Symbol> symbols;
    const State q0 = 0;
    mutable set<State> currentState;

public:
    eNKA() {} //kada je definiran drugi konstruktor default se treba definirati eksplicitno

    eNKA(const Grammar& grammar)
    {
        queue<LR1Item> generator;
        
        const Symbol& S = grammar.BEGIN_SYMBOL;
        symbols.emplace(S);
        
        //dodaje se jedina produkcija po pravilu a) iz skripte
        generator.emplace(
            S, grammar.PRODUKCIJE.at(S).at(0), set<Symbol>{end_sym}
        );
        
        while (!generator.empty()) 
        {
            LR1Item item = generator.front();
            generator.pop();

            State state = getState(item);

            while (!item.isComplete())
            {
                const Symbol& nextSym = item.symbolAfterDot();
                symbols.emplace(nextSym);

                //beta je niz znakova iza sljedeceg simbola kao u skripti
                const Word& beta = item.shift_dot_r().after_dot;
                //T je lookahead kao u skripti
                const set<Symbol> T = 
                    make_union (
                        grammar.startsWith(beta), 
                        (grammar.isVanishing(beta) ? item.lookahead : set<Symbol>{})
                    );

                //dodajem produkcije po pravilu c) str 148 iz skripte
                if (grammar.PRODUKCIJE.count(nextSym)) {
                    for (const Word& production : grammar.PRODUKCIJE.at(nextSym)) 
                    {
                        LR1Item nextItem (nextSym, production, T);
                        transitions[state][eps].insert(getState(nextItem));
                        generator.push(std::move(nextItem));
                    }
                }
                //dodajem produkciju po pravilu b) str 148 iz skripte
                transitions[state][nextSym].insert(state = getState(item));
            }    
        }

        bool evaluated[ID] = {};
        for (State state = q0; state < ID; state++)
            computeEpsilonEnvironment(state, grammar, evaluated);
        
        currentState = start();
    }

    //poc stanje
    inline set<State>& start() const {
        return transitions[q0][eps];
    }

    //eps okolina
    set<State> eps_of(const set<State>& current) const {
        return get_next(current, eps);
    }

    //prijelazi
    set<State> get_next(const set<State>& current, const Symbol& sym) const 
    {
        set<State> result;

        for (State state : current) 
            for (State next : transitions.at(state).at(sym))
                result.emplace(next);

        return result;
    }

    inline void reset() const {
        currentState = start();
    }

    inline void update(const Symbol& sym) const {
        currentState = get_next(currentState, sym);
    }

    const set<LR1Item> get_items() const {
        set<LR1Item> rez;
        for (State state : currentState) 
            rez.emplace(items[state]);
        return rez;
    } 

    inline std::size_t size() {
        return ID;
    }

private:
    set<State>& computeEpsilonEnvironment(State state, const Grammar& grammar, bool* evaluated)
    {
        set<State>& env = transitions[state][eps];

        if (!evaluated[state]) 
        {
            evaluated[state] = true;
            
            set<State> prev_env = transitions.at(state).at(eps);
            for (State next : prev_env)
                env = make_union(
                    env, computeEpsilonEnvironment(next, grammar, evaluated)
                );
        }

        return env;
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

private:
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

    inline const set<LR1Item>& get_items() const {
        return items.at(currentState);
    } 

    inline std::size_t size() const {
        return ID;
    }
};