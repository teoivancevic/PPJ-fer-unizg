#include <iostream>
#include <fstream>
#include "Utils.hpp"
// #include "Automat.hpp"
#include "Grammar.hpp"

//TODO:
class ParsingTable {

public:
    //(WARNING): varijable ne bi trebalo velkim slovom, ak možeš promjeni u camelCase
    
    map<pair<int, Symbol>, Action> akcija;  // (state, terminal) -> action
    map<pair<int, Symbol>, Action> novoStanje;  // (state, non-terminal) -> next state
    
    ParsingTable(const DKA& dka, const Grammar& grammar){
        dka.reset();
        // set<LR1Item> items = dka.items();

        for (int id_curr = 0; id_curr < dka.size(); id_curr++) { // prolazim kroz stanja DKA
            const map<Symbol, State> &prijelazi = dka.transitions.at(id_curr); // dohvacam prijelaze trenutnog stanja
            bool pomakIliStavi = false;

            for (const auto& [simbol_prijelaza, novo_stanje_id] : prijelazi) { // prolazim kroz prijelaze stanja
                if (grammar.ZAVRSNI.count(simbol_prijelaza)) { // ako je zavrsni znak
                    
                    // ako A -> A.aB stavka u itemima, simbol_prijelaza = a, ---> Pomak(stanje_id_sljedeceg_stanja)
                    // ako A -> A.aB, a nije u lookahead ---> Odbaci() --> ovo se nece dogoddit unutar ovog ifa?
                    // odmah dodaj u akcija, uvijek jednoznacno bcoz dka, itd.
                    akcija[{id_curr, simbol_prijelaza}] = "POMAKNI " + std::to_string(novo_stanje_id);
                    pomakIliStavi = true;
                } else { // ako je nezavrsni znak
                    novoStanje[{id_curr, simbol_prijelaza}] = "STAVI " + std::to_string(novo_stanje_id); // Stavi(stanje, )
                    pomakIliStavi = true;
                }
            }


            if (!pomakIliStavi) {
                set<LR1Item> current_items = dka.getItems(id_curr);
                
                for (const auto& item : current_items) {
                    if (item.after_dot.empty()) {
                        if (item.left == grammar.BEGIN_SYMBOL && item.lookahead.count(end_sym)) {
                            // Accept case
                            akcija[{id_curr, end_sym}] = "PRIHVATI";
                        } else {
                            // Reduce case - store the actual production
                            akcija[{id_curr, item.lookahead}] = Action(item.left, item.before_dot);
                        }
                    }
                }
            }


            // ako S' -> S. i $ je u lookahead ---> onda Prihvati() za (id_curr, $)
                //     PRIHVATI prioriteti, pazit ako tocka na desnom mjestu.
                //     edgevi vece prioriteti od onoga kj je u stanju
                //     ako vise edgeva onda prioriteti
                // ako A -> a. i B -> g je u lookahead ---> Reduciraj(A -> aBb)
                

            // trans = svi prijelazi iz trenutnog stanja
            // 
        //}

        throw "Not implemented";
    }; // TODO: fixat ovo

    void build();

    void outputToFile(const std::string& filename) const;

};


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
    grammar.dodajNoviPocetniZnak(GRAMMAR_NEW_BEGIN_STATE);
    if(DEBUG){
        grammar.dbgPrintFileLines();
        printf("\n");
        grammar.printInfo(); 
        cin.get();
    }
    
    
    

    // probaj inspectat s debuggerom (vrlo je cool)
    // map<LR1Item, int> m;
    // m[{"a", {}, {"a", "b", "c"}, {"b"}}] = 0;
    // m[{"a", {}, {"a", "c", "b"}, {"b"}}] = 1;
    // m[{"a", {}, {"a", "b", "c"}, {"b", "a"}}] = 2;
    // m[{"a", {}, {"a", "b", "c"}, {"a", "b"}}] = 3;
    // m[{"a", {"a"}, {"a", "b", "c"}, {"a"}}] = 4;
    // m[{"a", {}, {"a", "b", "c"}, {"b"}}] = 5;

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