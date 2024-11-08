#include <iostream>
#include <fstream>
#include "Utils.hpp"

int main () 
{   
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