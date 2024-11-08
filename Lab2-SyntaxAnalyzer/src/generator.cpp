#include <iostream>
#include <fstream>
#include "Utils.hpp"

/*vezano za using namespace std:
    radije ne bi to koristio jer je zbilja teško razlučit što je STL a što 
    naš kod. Umjesto tog koristi aliase kao ispod. 
*/

//ovo bi vjv trebalo definirati lokalno u klasama, ali se koriste dovoljno često izvan klasa pa je ok
using Symbol = std::string; //ovo treba razlikovati od std::string jer bi se trebalo koristiti samo u kontekstu gramatike (zato bi bilo bolje da je u klasi ali ok)
using State = int;
using Action = std::string; //--//--

//PROVJERI:
struct LR1Item 
{
    Symbol left;               
    vector<Symbol> before_dot;      
    vector<Symbol> after_dot;        
    Symbol lookahead;

    //nema potrebe definirati konstruktor za ovakav struct (aggregate type)
    
    bool operator==(const LR1Item& other) const {
        return left == other.left &&
               before_dot == other.before_dot &&
               after_dot == other.after_dot &&
               lookahead == other.lookahead;
    }
    
    bool operator<(const LR1Item& other) const {
        if (left != other.left) return left < other.left;
        if (before_dot != other.before_dot) return before_dot < other.before_dot;
        if (after_dot != other.after_dot) return after_dot < other.after_dot;
        return lookahead < other.lookahead;
    }

    //maknuo sam one cursed simbole sry, ak treba možeš vratit ali preferirao bi da izgleda ovako jer:
    //ako koristis stvari koje nisu u basic ASCII tablici postoji dobra sansa da se nece uvjek dobro ispisat
    std::string toString() const 
    { 
        std::string result = left + " -> ";
        for (const auto& s : before_dot) result += s + " ";
        result += ". ";
        for (const auto& s : after_dot) result += s + " ";
        result += ", " + lookahead;
        return result;
    }
};

template <>
struct std::hash<LR1Item>
{
    std::size_t operator()(const LR1Item& k) const
    {
        return ((std::hash<Symbol>()(k.left)
            ^ (std::hash<vector<Symbol>>()(k.before_dot) << 1)) >> 1)
            ^ (std::hash<vector<Symbol>>()(k.after_dot) << 1);
    }
};

//TODO:
class ParsingTable {
public:

    //(WARNING): varijable ne bi trebalo velkim slovom, ak možeš promjeni u camelCase
    
    map<pair<int, Symbol>, Action> Akcija;  // (state, terminal) -> action
    map<pair<int, State>, int> NovoStanje;  // (state, non-terminal) -> next state
    
    // ParsingTable(const DKA& dka, const Grammar& grammar){
    //     throw "Not implemented";
    // }; // TODO: fixat ovo

    void build();

    void outputToFile(const std::string& filename) const;
    // string getConflictReport() const;
};

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

//Dodo sam neka preimenovanja jer mi nisu bila konzistentna
//States bi trebala biti stanja automata, a Symbols znakovi gramatike, drzimo se toga

class Grammar
{
private:
    vector<std::string> fileLines_backup;
public:

    Symbol BEGIN_SYMBOL; // pocetni nezavrsni znak gramatike
    set<Symbol> NEZAVRSNI; // nezavrsni znakovi
    set<Symbol> ZAVRSNI; // zavrsni znakovi 
        //(note): mozda bolje drzat se engleskog TERMINATING
    set<Symbol> SYNC_ZAVRSNI; // sinkronizacijski zavrsni znakovi
    map<Symbol, vector<vector<Symbol>>> PRODUKCIJE;

    //code duplication
    Symbol readSymbol (const std::string line, set<Symbol>& container) {
        Symbol symbolsString = line.substr(3, line.size()-3);

        Symbol symbol = "";
        for(auto c: symbolsString){
            if(c == ' '){
                container.emplace(symbol);
                symbol = "";
            }else{
                symbol += c;
            }
        }
        container.emplace(symbol);

        return symbol;
    }

    // parse input file
    Grammar(std::string filePath) 
    {
        std::ifstream file(filePath);
        std::string line;

        if (file.is_open())
        {
            Symbol currSymbol = "";
            while(getline(file, line)){
                //cout << line << "\n";
                if(line[0] == '%')
                {
                    if(line[1] == 'V')
                        BEGIN_SYMBOL = readSymbol(line, NEZAVRSNI);
                    else if(line[1] == 'T') 
                        readSymbol(line, ZAVRSNI); 
                    else if(line.substr(1, 3) == "Syn") 
                        readSymbol(line, SYNC_ZAVRSNI);
                    else 
                        cerr << "Error in file\n";          
                }
                else if(line[0] == '<')
                {
                    currSymbol = line;
                }
                else if(line[0] == ' ')
                {
                    vector<Symbol> production = {};
                    Symbol symbolsString = line.substr(1, line.size()-1);

                    Symbol symbol = "";
                    for(auto c: symbolsString){
                        if(c == ' '){
                            production.emplace_back(symbol);
                            symbol = "";
                        }else{
                            symbol += c;
                        }
                    }
                    production.emplace_back(symbol);

                    PRODUKCIJE[currSymbol].push_back(production);
                }
                else
                {
                    /* code */
                    cerr << "Error in file\n";
                }

                fileLines_backup.push_back(line);
            }
            file.close();
        } 
        else
            cerr << "Unable to open file\n";
    }

    void dbgPrintFileLines () {
        for(auto l: fileLines_backup){
            cout << l << "\n";
        }
    }

    //Ovom GDB debugger moze posluzit, iako je bol za set upat
    void printInfo()
    {
        cout << "Nezavrsni: \t";
        for(auto s: NEZAVRSNI){
            cout << s << " ";
        }

        cout << "\nZavrsni: \t";
        for(auto z: ZAVRSNI)
            cout << z << " ";

        cout << "\nSync Zavrsni: \t";
        for(auto z: SYNC_ZAVRSNI)
            cout << z << " ";

        cout << "\nProdukcije: \n";
        for(auto p: PRODUKCIJE){
            cout << "  " << p.first << " ::= ";
            for(auto pp: p.second){
                for(auto z: pp)
                    if (z == "$")
                        cout << "\"\" ";
                    else
                        cout << z << " ";
                if(pp != p.second.back())
                    cout << "| ";
            }
            cout << "\n";
        }
    }

    void dodajNoviPocetniZnak ()
    {
        // dodajemo novi pocetni znak
        Symbol newStartSym = "<<q0>>";
        NEZAVRSNI.emplace(newStartSym);
        PRODUKCIJE[newStartSym] = {{BEGIN_SYMBOL}};
        BEGIN_SYMBOL = newStartSym;
    }
};

int main () 
{   
    //ne treba biti global var
    const std::string file_path = "../test/lab2_teza/19lr1/test.san";

    // korak 1 - parsiranje gramatike
    Grammar grammar(file_path);
    
    // korak 2 - dodajemo novi pocetni znak (zasto ovo nije u konstruktoru?)
    grammar.dodajNoviPocetniZnak();
    // grammar.printInfo();  

    // korak 3 - konstrukcija eNKA iz gramatike
    // eNKA enka(grammar);
    // enka.buildFromGrammar();

    // korak 4 - konstrukcija DKA iz eNKA
    // DKA dka(enka);

    // korak 5 - konstrukcija tablice parsiranja
    // ParsingTable table(dka, grammar); // TODO: fixat ovo
    // table.build(); // TODO: fixat ovo

    // korak 6 - ispis tablice parsiranja
    // table.outputToFile("parser_tables.txt"); // TODO: fixat ovo


    return 0;
}