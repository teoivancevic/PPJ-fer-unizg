#include<iostream>
#include <fstream>
#include <string>
#include <vector>
// #include <map>
// #include <set>
#include "Utils.hpp"

using namespace std;

// const string file_path = "../test/lab2_teza/00aab_1/test.san";
const string file_path = "../test/lab2_teza/19lr1/test.san";


// using State = string;
// using Symbol = string;

class LR1Item {
public:
    State left;               
    vector<string> before_dot;      
    vector<string> after_dot;        
    Symbol lookahead;                  
    
    LR1Item(State l, vector<string> before, vector<string> after, Symbol look)
        : left(l), before_dot(before), after_dot(after), lookahead(look) {}
    
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

    string toString() const {
        string result = left + " → ";
        for (const auto& s : before_dot) result += s + " ";
        result += "• ";
        for (const auto& s : after_dot) result += s + " ";
        result += ", " + lookahead;
        return result;
    }
};

class ParsingTable {
public:
    map<pair<int, Symbol>, string> Akcija;  // (state, terminal) -> action
    map<pair<int, State>, int> NovoStanje;        // (state, non-terminal) -> next state
    
    // ParsingTable(const DKA& dka, const Grammar& grammar){
    //     throw "Not implemented";
    // }; // TODO: fixat ovo

    void build();

    void outputToFile(const string& filename) const;
    // string getConflictReport() const;
};

class DKA;

// struct State {
    //     int id;
    //     set<LR1Item> items;
    //     map<string, set<int> > transitions;  // includes epsilon transitions
        
    //     bool operator==(const State& other) const;
    // };
class eNKA {
    

    friend DKA;
    //using Item = LR1Item;
    //nema potrebe stvarati state struct pogotovo ako ga stavis u vector
    using State = int;
    using Symbol = char; //I think? Ili je string? Ako je char onda makni const &
    template<typename T>
    using StateMap = map<State, T>;

    // StateMap<LR1Item> items; // vilim
    set<LR1Item> items; // teo
    // StateMap<map<Symbol, set<State>>> transitions;
    map<pair<LR1Item, Symbol>, set<LR1Item> > transitions;
    // set<State> start; // vilim
    set<LR1Item> start;
    // mutable set<State> currentState; // vilim
    set<LR1Item> currentState;

public:
    

    eNKA() {} //kada je definiran drugi konstruktor default se treba definirati eksplicitno

    eNKA(const Grammar& grammar) : grammar(grammar){
        //TODO
        string q0 = grammar.BEGIN_STATE;
        LR1Item item_q0 = LR1Item(q0, {}, grammar.PRODUKCIJE[q0][0], "$"); // ovo smije biti ovako po uputama labosa
        start.insert(item_q0);
        items.insert(item_q0);
        // start = closure(start);
        currentState = start;


        // TODO: teo dal trebam iz states pazit da ne dodam ovo stanje u enka
        // q0 -> S dot, $
        // prvo zelim napraviti "closure" za pocetno stanje
        for(auto grammarBegin: grammar.STATES){  // za svako nezavrsno stanje gramatike
            for(auto production: grammar.PRODUKCIJE[grammarBegin]){ // prodji kroz sve produkcije iz tog stanja
                for(int i = 0; i < production.size(); i++){ // za svaku produkciju prodji kroz sve znakove desne strane produkcije
                    vector<string> before_dot = {};
                    vector<string> after_dot = production; // cijela desna strana produkcije
                    
                    LR1Item item = LR1Item(grammarBegin, {}, after_dot, "$");
                    items.insert(item); // dodaj to stanje u skup stanja zao "root stanje" za taj odvojeni ogranak
                    string nextStateTrans = after_dot[0];

                    while (after_dot.empty() == false){
                        before_dot.push_back(after_dot[0]);
                        after_dot.erase(after_dot.begin());
                        LR1Item nextItem = LR1Item(grammarBegin, before_dot, after_dot, "$");
                        transitions[make_pair(item, nextStateTrans)].insert(nextItem);

                        // epsiloni iz stanja item u sva okolna
                        set<LR1Item> epsilonTransitions = computeEpsilonTransitions(nextItem);
                        for(auto epsilonTransition: epsilonTransitions){
                            transitions[make_pair(item, "")].insert(epsilonTransition);
                            items.insert(epsilonTransition); // todo: teo note: ovo potencijalno beskorisno
                        }

                        item = nextItem;
                        items.insert(item);
                    }
                    
                    
                }
            }
            
        }

        


    }

    void update (const Symbol& sym) const {
        //TODO
    }

    void reset () const {
        //TODO
    }

    set<Symbol> get_transitions(const set<State>& states) const {
        //TODO
    }

    set<State> get_next(const set<State>& current, const Symbol& sym) const {
        //TODO
    }

private:
    const Grammar& grammar;
    vector<State> states;
    int initial_state_id;

    set<LR1Item> closure(const set<LR1Item>& items);
    // set<LR1Item> computeEpsilonTransitions(const LR1Item& item);
    set<LR1Item> computeEpsilonTransitions(LR1Item item){
        set<LR1Item> result;
        queue<LR1Item> q;
        q.push(item);
        while (!q.empty()) {
            LR1Item current = q.front();
            q.pop();
            result.insert(current); // TODO: teo note: ovo ce dodati sve sudjedne u epsilon okolini, ali ce dodati i originalni item, nez dal to zelimo
            if (current.after_dot.empty()) continue;
            Symbol next = current.after_dot[0];
            if (grammar.STATES.count(next)) { // ako je nezavrsni znak prvi desni nakon dot-a
                for (const auto& production : grammar.PRODUKCIJE.at(next)) {
                    LR1Item next_item = LR1Item(next, {}, production, current.lookahead);
                    if (!result.count(next_item)) q.push(next_item); 
                    // todo: teo note: mislim da ce se s ovom linijom gore dodati svi sudjedni u epsilon okolini
                    // ali, mi ne radimo epsilon okolinu, nego samo epsilon prijelaze iz item u susjedne kojima je lijevo next i before_dot prazan
                }
            }
        }
        return result;

    }
    //int findStateId(const set<LR1Item>& items);
    State addState();
};

class DKA 
{
    friend eNKA;
    using Item = set<LR1Item>;
    using State = int;
    using Symbol = char;
    template<typename T>
    using StateMap = map<State, T>;

    StateMap<Item> items;
    StateMap<map<Symbol, State>> transitions;
    State start;
    mutable State currentState;

public:
    DKA() {}
    DKA(const eNKA& enka) 
    {
        enka.reset();
        SetMap<State> mapper;
        queue<set<State>> state_queue;
        state_queue.push(enka.start);
        State ID = 0;
        mapper[enka.start] = ID++;

        while (!state_queue.empty()) 
        {
            set<State> current = std::move(state_queue.front());
            State id = mapper[current];    
            state_queue.pop();

            for (State state : current)
                items[id].insert(enka.items.at(state));
                
            for (Symbol sym : enka.get_transitions(current)) 
            {
                set<State> next = enka.get_next(current, sym);
                if (!mapper.count(next)) {
                    transitions[id][sym] = ID;
                    mapper[next] = ID++;
                    state_queue.emplace(std::move(next));
                }
            }
        }
    }    
};




class Grammar{
private:
    vector<string> fileLines_backup;
public:
    

    vector<State> STATES; // nezavrsni znakovi
    State BEGIN_STATE; // pocetni nezavrsni znak gramatike
    vector<Symbol> ZAVRSNI; // zavrsni znakovi
    vector<Symbol> SYNC_ZAVRSNI; // sinkronizacijski zavrsni znakovi
    map<State, vector<vector<string>>> PRODUKCIJE;

    // parse input file
    Grammar(string filePath){
        ifstream file(file_path);
        string line;

        if(file.is_open()){
            State currState = "";
            while(getline(file, line)){
                //cout << line << "\n";
                if(line[0] == '%'){
                    if(line[1] == 'V')
                    {
                        /*  Nezavrsni znakovi
                            Format linije: 
                            %V <Program> <Naredba> <primarni_izraz>

                            U prikazanom primjeru, <Program> je pocetni nezavrsni znak gramatike.
                        */

                        string statesString = line.substr(3, line.size()-3);
                        vector<State> states = {}; 

                        string state = "";
                        for(auto c: statesString){
                            if(c == ' '){
                                STATES.push_back(state);
                                state = "";
                            }else{
                                state += c;
                            }
                        }
                        STATES.push_back(state);

                        BEGIN_STATE = STATES[0];
                    }
                    else if(line[1] == 'T')
                    {
                        /*  Zavrsni znakovi
                            Format linije: 
                            %T IDENTIFIKATOR brojcanaKonstanta znakovnaKonstanta OP_PLUS
                        */

                        string symbolsString = line.substr(3, line.size()-3);
                        string symbol = "";
                        for(auto c: symbolsString){
                            if(c == ' '){
                                ZAVRSNI.push_back(symbol);
                                symbol = "";
                            }else{
                                symbol += c;
                            }
                        }
                        ZAVRSNI.push_back(symbol);
                        
                    }
                    else if(line.substr(1, 3) == "Syn"){
                        // sinkronizacijski zavrsni znakovi
                        string sync_symbolsString = line.substr(5, line.size()-5);
                        string symbol = "";
                        for(auto c: sync_symbolsString){
                            if(c == ' '){
                                SYNC_ZAVRSNI.push_back(symbol);
                                symbol = "";
                            }else{
                                symbol += c;
                            }
                        }
                        SYNC_ZAVRSNI.push_back(symbol);
                    }
                    else
                    {
                        cerr << "Error in file\n";   
                    }
                        
                }
                else if(line[0] == '<')
                {
                    currState = line;
                }
                else if(line[0] == ' ')
                {
                    line = line.substr(1, line.size()-1);
                    vector<string> production = {};
                    string symbol = "";
                    for(auto c: line){
                        if(c == ' '){
                            production.push_back(symbol);
                            symbol = "";
                        }else{
                            symbol += c;
                        }
                    }
                    production.push_back(symbol);

                    PRODUKCIJE[currState].push_back(production);
                }
                else
                {
                    /* code */
                    cerr << "Error in file\n";
                }
                


                fileLines_backup.push_back(line);
            }
            file.close();
        }else{
            cerr << "Unable to open file\n";
        }
    }

    void dbgPrintFileLines(){
        for(auto l: fileLines_backup){
            cout << l << "\n";
        }
    }

    void printInfo(){

        cout << "Nezavrsni: \t";
        for(auto s: STATES){
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

    void dodajNoviPocetniZnak(){
        // dodajemo novi pocetni znak
        string newStartState = "<<q0>>";
        STATES.push_back(newStartState);
        PRODUKCIJE[newStartState] = {{BEGIN_STATE}};
        BEGIN_STATE = newStartState;
    }
};

int main(){
    
    // korak 1 - parsiranje gramatike
    Grammar grammar(file_path);
    
    // korak 2 - dodajemo novi pocetni znak
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