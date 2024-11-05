#include<iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>

using namespace std;

// const string file_path = "../test/lab2_teza/00aab_1/test.san";
const string file_path = "../test/lab2_teza/19lr1/test.san";


using State = string;
using Symbol = string;

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

class eNKA {
public:
    struct State {
        int id;
        set<LR1Item> items;
        map<string, set<int> > transitions;  // includes epsilon transitions
        
        bool operator==(const State& other) const;
    };

    eNKA(const Grammar& grammar) : grammar(grammar), initial_state_id(0) {
        createInitialState(); // LR1 stavka iz produkcije s novim pocetnim znakom
    }

    void createInitialState(){
        // create initial state

    }

    void buildFromGrammar();
    // todo: vidit je li ovo potrebno
    // const vector<State>& getStates() const;
    // int getInitialStateId() const;

private:
    const Grammar& grammar;
    vector<State> states;
    int initial_state_id;

    set<LR1Item> closure(const set<LR1Item>& items);
    set<LR1Item> computeEpsilonTransitions(const LR1Item& item);
    int findStateId(const set<LR1Item>& items);
    int addState(const set<LR1Item>& items);
};

class DKA {
public:
    struct State {
        int id;
        set<LR1Item> items;
        map<string, int> transitions;
    };

    DKA(const eNKA& enka);
    void convertFromENKA();
    const vector<State>& getStates() const;
    int getInitialStateId() const;

private:
    const eNKA& enka;
    vector<State> states;
    int initial_state_id;
    map<set<int>, int> subset_map;
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