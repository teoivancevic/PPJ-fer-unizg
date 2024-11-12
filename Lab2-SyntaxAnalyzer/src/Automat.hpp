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
    State q0;

//DEBUG
    mutable set<State> currentState;
    mutable set<LR1Item> currentItems;

public:
    eNKA() {}

    eNKA(const Grammar& grammar)
    {
        queue<State> generator;
        
        const Symbol& S = grammar.BEGIN_SYMBOL;
        symbols.emplace(S);
        
        //dodaje se jedina produkcija po pravilu a) iz skripte
        generator.emplace(q0 = getState(
            {S, grammar.PRODUKCIJE.at(S).at(0), {end_sym}}
        ));
        
        //za svaku stavku rekurzivno generiranu iz početne:
        while (!generator.empty()) 
        {

            //dohvati se sljedeći stateID
            State state = generator.front();

            //dohvati se pripadni LR1Item
            LR1Item item = items.at(state);
            generator.pop();
            // cerr << "item loaded " << std::endl;
//             LR1Item item = generator.front();
//             generator.pop();
//             cerr << "item loaded " << std::endl;
//             State state = getState(item);
//             cerr << "state: " << state << std::endl;

            //za svaku stavku dobivenu shiftanjem točke u desno:

            while (!item.isComplete())
            {
                //dohvati sljedeći znak iza točke
                const Symbol& nextSym = item.symbolAfterDot();
                symbols.emplace(nextSym);

                //beta je niz znakova iza sljedeceg simbola kao u skripti str 148
                const Word& beta = item.shift_dot_r().after_dot;
                //T je lookahead kao u skripti
                const set<Symbol> T = 
                    make_union ( //računato po pravilima i) ii) na str 148
                        grammar.startsWith(beta), 
                        (grammar.isVanishing(beta) ? item.lookahead : set<Symbol>{})
                    );

                //dodajem produkcije po pravilu c) str 148 iz skripte
                if (grammar.PRODUKCIJE.count(nextSym)) {
                    for (const Word& production : grammar.PRODUKCIJE.at(nextSym)) 
                    {
                        State nextState = getState({nextSym, production, T});
                        //dodaje se prijelaz
                        transitions[state][eps].insert(nextState);
                        //sljedece stanje se stavlja u queue za evaluaciju
                        if (nextState == ID-1) generator.push(nextState);
                    }
                }
              
                cerr << "enka before insert transition" << std::endl;
                //dodajem produkciju po pravilu b) str 148 iz skripte
                transitions[state][nextSym].insert(state = getState(item));
                cerr << "enka AFTER insert transition" << std::endl;
            }    
        }
        cerr << "eNKA constructed, from enka" << std::endl;

        //izračunaj sva eps okruženja
        bool evaluated[ID] = {};
        for (State state = q0; state < ID; state++)
            computeEpsilonEnvironment(state, grammar, evaluated);
        
        reset();
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
            if (exists_trans(state, sym))
            for (State next : transitions.at(state).at(sym))
                result.emplace(next);

        return result;
    }

    inline void reset() const {
        currentState = start();
        currentItems = get_items();
    }

    inline void update(const Symbol& sym) const {
        currentState = eps_of(get_next(currentState, sym));
        currentItems = get_items();
    }

    const set<LR1Item> get_items() const {
        set<LR1Item> rez;
        for (State state : currentState) 
            rez.emplace(items[state]);
        return rez;
    } 

    //provjera za mape
    inline bool exists_trans(State state, const Symbol& sym) const {
        return exists(transitions, state) && exists(transitions.at(state), sym);
    }

    inline std::size_t size() {
        return ID;
    }

private:
    set<State>& computeEpsilonEnvironment(State state, const Grammar& grammar, bool* evaluated)
    {
        if (!exists_trans(state, eps)) {
            evaluated[state] = true;
            return transitions[state][eps] = set<State>{state};
        }
        
        set<State>& env = transitions.at(state).at(eps);

        if (!evaluated[state]) 
        {
            evaluated[state] = true;
            
            set<State> prev_env = transitions.at(state).at(eps);
            for (State next : prev_env)
                env = make_union(
                    env, computeEpsilonEnvironment(next, grammar, evaluated)
                );
            
            env.insert(state);
        }

        return env;
    }

    //ovo ili vrati postojeci state il ga na pravi pa vrati
    State getState(const LR1Item& item) {
        if (!states.count(item)) {
            items[ID] = item;
            states[item] = ID++;
        }
        return states[item];
    }
};

class DKA 
{
    State ID = 0;

public:
    template<typename T>
    using StateMap = map<State, T>;
    const State start;
    mutable State currentState;

// private:
    StateMap<set<LR1Item>> items;
    StateMap<map<Symbol, State>> transitions;

//DEBUG
    mutable set<LR1Item> currentItems;

public:
    DKA() : start(ID) {}

    DKA(const eNKA& enka) : start(ID)
    {
        SetMap<State> mapper;
        queue<set<State>> state_queue;

        mapper[enka.start()] = ID++;
        state_queue.push(enka.start());

        //za svaki pronađeni skup stanja u enka:
        while (!state_queue.empty()) 
        {
            //dohvati sljedeći skup na redu
            set<State> current = std::move(state_queue.front());
            state_queue.pop();

            //dohvati DKA id
            State id = mapper.at(current);

            //dohvati sve LR1Iteme
            for (State state : current)
                items[id].insert(enka.items.at(state));
            
            //pronađi sve prijelaze
            for (const Symbol& sym : enka.symbols) 
            {
                set<State> next = enka.eps_of(enka.get_next(current, sym));
                
                if (!next.empty()) 
                {
                    //ako je već zabilježeno stanje samo dodaj prijelaz, inaće obradi novo stanje
                    if (exists(mapper, next))
                        transitions[id][sym] = mapper.at(next);
                    else {
                        transitions[id][sym] = ID;
                        mapper[next] = ID++;
                        state_queue.emplace(std::move(next));
                    }
                }
            }
        }

        reset();
    }

    inline void reset() const {
        currentState = start;
        currentItems = get_items();
    }

    inline void update(const Symbol& sym) const 
    {
        currentState = (exists_trans(currentState, sym) ? transitions.at(currentState).at(sym) : -1);
        currentItems = get_items();
    }

    inline const set<LR1Item> get_items() const {
        return (exists(items, currentState) ? items.at(currentState) : set<LR1Item>{});
    }

    inline const set<LR1Item>& itemsAtState(State state) const {
        return items.at(state);
    } 

    inline std::size_t size() const {
        return ID;
    }

    //provjera za mape
    inline bool exists_trans(State state, const Symbol& sym) const {
        return exists(transitions, state) && exists(transitions.at(state), sym);
    }
};