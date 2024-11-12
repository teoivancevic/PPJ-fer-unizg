#include <iostream>
#include <fstream>
#include "Utils.hpp"
#include "Automat.hpp"
#include "Grammar.hpp"


class ParsingTable {

public:
    
    map<pair<int, Symbol>, Action> akcija;  // (state, terminal) -> action
    map<pair<int, Symbol>, Action> novoStanje;  // (state, non-terminal) -> next state
    
    ParsingTable() {}

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

    void outputToFile(const std::string& filename, const Grammar& grammar) const {
        std::ofstream out(filename);
        
        out << "SYNC_SYMBOLS:\n";
        for (const auto& symbol : grammar.SYNC_ZAVRSNI) {
            out << symbol << "\n";
        }

        out << "GRAMMAR_PRODUCTIONS:\n";
        for (int i=0; i < grammar.ID_global; i++) {
            auto [left, right] = grammar.ID_PRODUKCIJE_MAPA.at(i);
            out << i << " " << left << " -> ";
            for (const auto& s : right) {
                if(s == right.back()) 
                    out << s;
                else
                    out << s << " ";
            }
            out << "\n";
        }

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


bool DEBUG = true;

// ne brisat ovo, koristit cu za testiranje
void teoMain(){
    // korak 1 - parsiranje gramatike
    Grammar grammar("cin");
    cerr << "Grammar parsed" << std::endl;
    // korak 2 - dodajemo novi pocetni znak (zasto ovo nije u konstruktoru?)
    grammar.dodajNoviPocetniZnak(GRAMMAR_NEW_BEGIN_STATE);
    cerr << "New start symbol added" << std::endl;
    if (DEBUG){
        grammar.dbgPrintFileLines();
        printf("\n");
        grammar.printInfo(); 
        // cin.get();
    }
    cerr << "Grammar info printed" << std::endl;
    // korak 3 - konstrukcija eNKA iz gramatike
    eNKA enka(grammar);
    cerr << "eNKA constructed" << std::endl;
    // korak 4 - konstrukcija DKA iz eNKA
    DKA dka(enka);
    cerr << "DKA constructed" << std::endl;
    // korak 5 - konstrukcija tablice parsiranja
    ParsingTable table(dka, grammar); // TODO: fixat ovo
    cerr << "Parsing table constructed" << std::endl;
    // korak 6 - ispis tablice parsiranja
    table.outputToFile("analizator/tablica.txt", grammar); // TODO: fixat ovo
}

void teoMain_mockParsingTable(){
    std::string filePath = "../test/lab2_teza/00aab_1/test.san";
    
    // Grammar grammar("cin");
    Grammar grammar(filePath);
    grammar.dodajNoviPocetniZnak(GRAMMAR_NEW_BEGIN_STATE);
    if(DEBUG){
        grammar.dbgPrintFileLines();
        printf("\n");
        grammar.printInfo();
    }
    cerr << "Grammar parsed" << std::endl;
    ParsingTable table;

    // note: ova tablica je glupost napisana, samo kako bi se testirao output u txt file
    //      i da se onda moze testirat parsiranje iz txt filea u analizatoru
    table.akcija[{0, "a"}] = "POMAKNI 2";    // on 'a', shift to state 2
    table.akcija[{0, "b"}] = "POMAKNI 3";    // on 'b', shift to state 3
    table.novoStanje[{0, "S"}] = "STAVI 1";  // on S, goto state 1
    table.novoStanje[{0, "A"}] = "STAVI 4";  // on A, goto state 4
    table.novoStanje[{0, "B"}] = "STAVI 5";  // on B, goto state 5

    // State 1: after seeing <S>
    table.akcija[{1, "$"}] = "PRIHVATI";     // accept if we see end marker

    // State 2: after seeing 'a'
    table.akcija[{2, "a"}] = "REDUCIRAJ 1";  // reduce A ::= a
    table.akcija[{2, "b"}] = "REDUCIRAJ 1";  // reduce A ::= a
    table.akcija[{2, "$"}] = "REDUCIRAJ 1";  // reduce A ::= a

    // State 3: after seeing 'b'
    table.akcija[{3, "a"}] = "REDUCIRAJ 2";  // reduce B ::= b
    table.akcija[{3, "b"}] = "REDUCIRAJ 2";  // reduce B ::= b
    table.akcija[{3, "$"}] = "REDUCIRAJ 2";  // reduce B ::= b

    // State 4: after seeing <A>
    table.akcija[{4, "a"}] = "POMAKNI 2";    // on 'a', shift to state 2
    table.novoStanje[{4, "A"}] = "STAVI 6";  // on A, goto state 6

    // State 5: after seeing <B>
    table.akcija[{5, "a"}] = "REDUCIRAJ 6";  // reduce S ::= B
    table.akcija[{5, "b"}] = "REDUCIRAJ 6";  // reduce S ::= B
    table.akcija[{5, "$"}] = "REDUCIRAJ 6";  // reduce S ::= B
    table.novoStanje[{5, "S"}] = "STAVI 7";  // on S, goto state 7

    // State 6: after seeing <A><A>
    table.akcija[{6, "a"}] = "POMAKNI 2";    // on 'a', shift to state 2
    table.akcija[{6, "b"}] = "POMAKNI 3";    // on 'b', shift to state 3
    table.novoStanje[{6, "S"}] = "STAVI 8";  // on S, goto state 8

    // State 7: after seeing <B><S>
    table.akcija[{7, "a"}] = "REDUCIRAJ 4";  // reduce S ::= B S
    table.akcija[{7, "b"}] = "REDUCIRAJ 4";  // reduce S ::= B S
    table.akcija[{7, "$"}] = "REDUCIRAJ 4";  // reduce S ::= B S

    // State 8: after seeing <A><A><S>
    table.akcija[{8, "a"}] = "REDUCIRAJ 3";  // reduce S ::= A A S
    table.akcija[{8, "b"}] = "REDUCIRAJ 3";  // reduce S ::= A A S
    table.akcija[{8, "$"}] = "REDUCIRAJ 3"; 
    
    cerr << "Parsing table constructed" << std::endl;
    
    table.outputToFile("analizator/tablica.txt", grammar);
}

int main () 
{
    // teoMain();
    teoMain_mockParsingTable();

   
    std::string file_path = "../test/lab2_teza/01aab_2/test.san";

    Grammar grammar(file_path);
    

    // korak 2 - dodajemo novi pocetni znak (zasto ovo nije u konstruktoru?)

    // grammar.dodajNoviPocetniZnak(GRAMMAR_NEW_BEGIN_STATE);
  
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