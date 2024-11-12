#include <iostream>
#include <fstream>
#include "Utils.hpp"
#include "Automat.hpp"
#include "Grammar.hpp"

//TODO:
class ParsingTable {

public:
    //(WARNING): varijable ne bi trebalo velkim slovom, ak možeš promjeni u camelCase
    
    map<pair<int, Symbol>, Action> akcija;  // (state, terminal) -> action
    map<pair<int, Symbol>, Action> novoStanje;  // (state, non-terminal) -> next state
    
    ParsingTable (const DKA& dka, const Grammar& grammar){
        dka.reset();
        // set<LR1Item> items = dka.items();

        for (int id_curr = 0; id_curr < dka.size(); id_curr++) { // prolazim kroz stanja DKA
            const map<Symbol, State> &prijelazi = dka.transitions.at(id_curr); // dohvacam prijelaze trenutnog stanja
            bool pomakIliStavi = false; // rjesava nejednoznacnost POMAKNI/REDUCIRAJ

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
                set<LR1Item> current_items = dka.itemsAtState(id_curr);

                map<Symbol, vector<pair<int, LR1Item>>> reductions_by_lookahead;
                
                for (const auto& item : current_items) {
                    if (item.after_dot.empty()) {
                        if (item.left == grammar.BEGIN_SYMBOL && item.lookahead.count(end_sym)) {
                            // Accept case
                            // S' -> s. {$}
                            akcija[{id_curr, end_sym}] = "PRIHVATI";
                        } else {
                            // Reduce case - store the actual production
                            // instead of actual production store id, from the order of input in grammar
                            
                            int prod_index = grammar.ID_PRODUKCIJE.at({item.left, item.before_dot});
                            // TODO: rijesiti nejednoznacnost reduciraj/reduciraj
                            // prioritet ima produkcija s manjim ID-jem produkcije
                            if (prod_index != -1) {
                                
                                
                                // for (const auto& look : item.lookahead) {
                                //     akcija[{id_curr, look}] = "REDUCIRAJ " + std::to_string(prod_index);
                                // }
                                for (const auto& look : item.lookahead) {
                                    reductions_by_lookahead[look].push_back({prod_index, item});
                                }
                            }

                            // "REDUCIRAJ j" --> j je id produkcije u gramatici
                        }
                    }
                }

                // nejednoznacnost?
                for (auto& [lookahead, reductions] : reductions_by_lookahead) {
                    if (reductions.size() > 1) {
                        // Sort by production ID (lower ID has priority)
                        sort(reductions.begin(), reductions.end(), 
                             [](const auto& a, const auto& b) { return a.first < b.first; });
                        
                        // Log conflict
                        cerr << "Reduce/Reduce conflict in state " << id_curr 
                             << " for lookahead " << lookahead << std::endl;
                        cerr << "Choosing production with ID " << reductions[0].first 
                             << " over productions with IDs: ";
                        for (size_t i = 1; i < reductions.size(); i++) {
                            cerr << reductions[i].first << " ";
                        }
                        cerr << std::endl;
                    }
                    
                    // Use the reduction with lowest ID
                    // Store just "REDUCIRAJ <index>"
                    akcija[{id_curr, lookahead}] = "REDUCIRAJ " + std::to_string(reductions[0].first);
                }
            }


            
                
        }

    }; 

    // ako S' -> S. i $ je u lookahead ---> onda Prihvati() za (id_curr, $)
        //     PRIHVATI prioriteti, pazit ako tocka na desnom mjestu.
        //     edgevi vece prioriteti od onoga kj je u stanju
        //     ako vise edgeva onda prioriteti
        // ako A -> a. i B -> g je u lookahead ---> Reduciraj(A -> aBb)

    // void build();

    void outputToFile(const std::string& filename) const {
        std::ofstream out(filename);
        
        out << "SYNC_SYMBOLS:\n";


        out << "GRAMMAR_PRODUCTIONS:\n";


        out << "AKCIJA:\n";
        for (const auto& [key, value] : akcija) {
            out << key.first << " " << key.second << " " << value << "\n";
        }
        
        out << "NOVO STANJE:\n";
        for (const auto& [key, value] : novoStanje) {
            out << key.first << " " << key.second << " " << value << "\n";
        }
        
        out.close();
    }

};


bool DEBUG = false;

int main () 
{
    // DEBUG = true;
    std::string file_path = "../test/lab2_teza/01aab_2/test.san";

    Grammar grammar(file_path);
    

    // korak 2 - dodajemo novi pocetni znak (zasto ovo nije u konstruktoru?)

    grammar.dodajNoviPocetniZnak(GRAMMAR_NEW_BEGIN_STATE);
    
    if (DEBUG){
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