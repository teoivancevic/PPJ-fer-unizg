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
    std::string file_path = "../test/lab2_teza/19lr1/test.san";
    // std::string file_path;
    //cin >> file_path;

    // korak 1 - parsiranje gramatike
    Grammar grammar(file_path);
    // Grammar grammar("cin");
    
    // korak 2 - dodajemo novi pocetni znak (zasto ovo nije u konstruktoru?)
    grammar.dodajNoviPocetniZnak("<S'>");
    if(DEBUG){
        grammar.dbgPrintFileLines();
        printf("\n");
        grammar.printInfo(); 
        // cin.get();
    }

    eNKA enka(grammar);
    
    // cout <<"\n" <<grammar.startsWith(vector<Symbol>{"<A>", "<B>"}, "a") <<std::endl;

    // cout <<grammar.isVanishing(eps) <<std::endl; 
    // auto a = grammar.startsWith("<A>");

    return 0;

    cin.get();
    
    //ne treba biti global var
    // const std::string file_path = "../test/lab2_teza/19lr1/test.san";

    // korak 1 - parsiranje gramatike
    // Grammar grammar(file_path);
    
    // korak 2 - dodajemo novi pocetni znak (zasto ovo nije u konstruktoru?)
    // grammar.dodajNoviPocetniZnak();
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