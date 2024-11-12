#include <iostream>
#include <fstream>
#include "Utils.hpp"
#include "Automat.hpp"
#include "Grammar.hpp"

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


bool DEBUG = false;

int main () 
{
    // DEBUG = true;
    std::string file_path = "../test/lab2_teza/01aab_2/test.san";

    Grammar grammar(file_path);
    
    grammar.dodajNoviPocetniZnak("<S'>");
    if (DEBUG) {
        grammar.dbgPrintFileLines();
        printf("\n");
        grammar.printInfo();
    }

    eNKA enka(grammar);
    DKA dka(enka);

    std::string in = "b <B> a <S> b a b <A>";
    // cin >>in;

    forEachWord(in, [&dka, &enka](const Symbol& sym){
        enka.update(sym);
        dka.update(sym);
    });

    return 0;

    return 0;
}