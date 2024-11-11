#include <iostream>
#include <fstream>
#include "Utils.hpp"
// #include "Automat.hpp"
#include "Grammar.hpp"

int main () 
{
    const bool DEBUG = true;
    
    std::string file_path = "../test/lab2_teza/19lr1/test.san";
    // std::string file_path;
    //cin >> file_path;

    // korak 1 - parsiranje gramatike
    // Grammar grammar(file_path);
    Grammar grammar("cin");
    
    // korak 2 - dodajemo novi pocetni znak (zasto ovo nije u konstruktoru?)
    grammar.dodajNoviPocetniZnak("<<S'>>");
    if(DEBUG){
        grammar.dbgPrintFileLines();
        printf("\n");
        grammar.printInfo(); 
        cin.get();
    }
    
    
    

    // probaj inspectat s debuggerom (vrlo je cool)
    map<LR1Item, int> m;
    m[{"a", {}, {"a", "b", "c"}, {"b"}}] = 0;
    m[{"a", {}, {"a", "c", "b"}, {"b"}}] = 1;
    m[{"a", {}, {"a", "b", "c"}, {"b", "a"}}] = 2;
    m[{"a", {}, {"a", "b", "c"}, {"a", "b"}}] = 3;
    m[{"a", {"a"}, {"a", "b", "c"}, {"a"}}] = 4;
    m[{"a", {}, {"a", "b", "c"}, {"b"}}] = 5;

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