#include "Utils.hpp"
#include "Grammar.hpp"

//Kužim što si radio s LRItem, ali da pojednostavimo stvari, lakše je mapirati ih u brojeve i onda koristiti njih kao State

//PROVJERI:
class eNKA 
{
    State ID = 0;

public:
    static const Symbol eps;

    template<typename T>
    using StateMap = map<State, T>;
        //ovo bi mogo biti vektor, ali da izbjegnemo spammanje push_back() i 
        //potencijalan illegal mem access ovo je safer, plus lakše je adaptirat ako State nije int

//READ ONLY, public za easy access
    mutable map<LR1Item, State> states; //pretvorba LR1Item -> State
    mutable StateMap<LR1Item> items;
    mutable StateMap<map<Symbol, set<State>>> transitions;
    mutable set<Symbol> symbols;
    mutable State startState;

public:
    set<State>& start() const {
        return transitions[startState][eps];
    }

    eNKA() {} //kada je definiran drugi konstruktor default se treba definirati eksplicitno

    eNKA(const Grammar& grammar)
    {
        //(note): pazi kad koristis at() a kada operator[] s mapama!

        const Symbol& S = grammar.BEGIN_SYMBOL; //S je prikladnija oznaka za simbol od q0
        startState = newState({S, {}, grammar.PRODUKCIJE.at(S)[0], eps});

        //at() koristi kada god je moguće aka. kada postoji zapis i samo ga citas

        for(auto grammarBegin: grammar.NEZAVRSNI) // za svako nezavrsno stanje gramatike
        for(const auto& production: grammar.PRODUKCIJE.at(grammarBegin)) // prodji kroz sve produkcije iz tog stanja
        for(int i = 0; i < production.size(); i++) // za svaku produkciju prodji kroz sve znakove desne strane produkcije
        { 
            vector<Symbol> before_dot = {};
            vector<Symbol> after_dot = production; // cijela desna strana produkcije

            State state = getState({
                grammarBegin, before_dot, after_dot, eps
            });
            
            start().insert(state); // dodaj to stanje u skup stanja za "root stanje" za taj odvojeni ogranak
            Symbol nextStateTrans = after_dot[0];

            while (!after_dot.empty())
            {
                before_dot.push_back(after_dot[0]);
                after_dot.erase(after_dot.begin());
                
                LR1Item next_item = {
                    grammarBegin, before_dot, after_dot, eps
                };
                State next_state = getState(next_item);

                transitions[state][nextStateTrans].insert(next_state);

                // epsiloni iz stanja item u sva okolna
                for(const State& epsilonTransition: computeEpsilonTransitions(next_item, grammar))
                    transitions[state][eps].emplace(epsilonTransition);

                state = next_state;
            }      
        }
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

private:
    set<State> computeEpsilonTransitions(const LR1Item& item, const Grammar& grammar)
    {
        set<State> result;
        queue<LR1Item> q;
        q.push(item);
        
        while (!q.empty()) 
        {
            LR1Item current = q.front();
            q.pop();

            result.insert(getState(item)); // TODO: teo note: ovo ce dodati sve sudjedne u epsilon okolini, ali ce dodati i originalni item, nez dal to zelimo 
                //(vilim note: zelimo)
            
            if (current.after_dot.empty()) 
                continue;
            Symbol next = current.after_dot[0];
            
            if (grammar.NEZAVRSNI.count(next)) // ako je nezavrsni znak prvi desni nakon dot-a
            { 
                for (const auto& production : grammar.PRODUKCIJE.at(next)) {
                    LR1Item next_item = LR1Item(next, {}, production, current.lookahead);
                    if (!result.count(getState(next_item))) 
                        q.push(next_item); 
                }
            }
        }

        return result;
    }

    State newState(const LR1Item& item) {
        if (states.count(item)) {
            printf("duplicate state: %s\n", item.toString()); //ne bi se trebalo dogoditi i think
            return -1;
        }
        return states[item] = ID++;
    }

    State getState(const LR1Item& item) {
        if (states.count(item))
            return states[item];
        return newState(item);
    }

    std::size_t size() {
        return ID;
    }
};

const Symbol eNKA::eps = "$";

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

    void reset();

    bool update(const Symbol& sym);

    std::size_t size() {
        return ID;
    }
};