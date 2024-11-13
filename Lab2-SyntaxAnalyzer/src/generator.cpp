#include <iostream>
#include <fstream>
#include "Utils.hpp"
#include "Automat.hpp"
#include "Grammar.hpp"

bool DEBUG = false;

struct Action {
    std::string name;
    int id;

    Action() : name("ODBACI"), id(0) {}

    Action(std::string name, int id) : name(name), id(id) {}

    Action(std::string str) {
        *this = std::move(str);
    }

    Action &operator= (std::string str) {
        name = consumeNextWord(str);
        id = to_int(consumeNextWord(str));
        return *this;
    }

    std::string toString() const {
        return name + " " + std::to_string(id);
    }
};

struct ParsingTable 
{
    map<pair<State, Symbol>, Action> akcija;  // (state, terminal) -> action
    map<pair<State, Symbol>, Action> novoStanje;  // (state, non-terminal) -> next state
    
    ParsingTable() {}

    ParsingTable (const DKA& dka, const Grammar& grammar)
    {
        for (State current = 0; current < (int) dka.size(); current++) 
        {     
            //za svaku stavku u stanju:
            for (LR1Item item : dka.items.at(current)) 
            {
                const Symbol& sym = item.symbolAfterDot();
                const auto key = pair{current, sym};
                
                //provjerava vrijedi li pravilo c) iz udzb str 151
                if (item.left == GRAMMAR_NEW_BEGIN_STATE && sym == end_sym) 
                {
                    akcija.emplace(key, Action{"PRIHVATI"});
                    continue;
                }
                //provjera vrijedi li pravilo a) --//--
                if (dka.exists_trans(current, sym)) 
                {
                    State nextState = dka.transitions.at(current).at(sym);
                    
                    if (grammar.isTerminating(sym))
                        akcija.emplace(key, Action{"POMAKNI", nextState});
                    else 
                        novoStanje.emplace(key, Action{"STAVI", nextState});
                }   
                //provjerava vrijedi li pravilo b) --//-- ... uz dodatni uvjet prednosti iz uputa labosa
                if (item.isComplete()) 
                {
                    //kako bi item.after_dot bio jednak produkciji
                    while (!item.before_dot.empty()) 
                        item.shift_dot_l();
                    
                    //id produkcije
                    int id = grammar.ID_PRODUKCIJE.at({item.left, item.after_dot});

                    for (const Symbol& lookahead : item.lookahead) 
                    {
                        const auto key = pair{current, lookahead};
                        //razrjesavanje nejednoznacnosti
                        if (exists(akcija, key) && akcija.at(key).name != "POMAKNI" && akcija.at(key).id > id)
                            akcija[key] = Action{"REDUCIRAJ", id};
                        else if (!exists(akcija, key))
                            akcija.emplace(key, Action{"REDUCIRAJ", id});
                    }
                }     
            }
        }
    }; 

    void outputToFile(const std::string& filename, const Grammar& grammar) const 
    {
        std::ofstream out(filename);
        
        out << "SYNC_SYMBOLS:" <<endl;
        for (const Symbol& symbol : grammar.SYNC_ZAVRSNI)
            out << symbol << endl;

        out << "\nGRAMMAR_PRODUCTIONS:" <<endl;
        for (const auto& [production, id] : grammar.ID_PRODUKCIJE) 
            out << id << " " <<production.first << " -> " << concatToString_r(production.second) << endl; //REVERSE

        out << "\nAKCIJA:" <<endl;
        for (const auto& [key, action] : akcija)
            out << key.first << " " << key.second << " " << action.toString() << endl;
        
        out << "\nNOVO STANJE:" <<endl;
        for (const auto& [key, action] : novoStanje)
            out << key.first << " " << key.second << " " << action.toString() << endl;

        
        out.close();
    }
};

void teoMain_mockParsingTable()
{
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

std::string input_file = "../test/21lr1/test.san";

int main ()
{
    // korak 1 - parsiranje gramatike
    // input_file = "cin";
    Grammar grammar(input_file);
    
    // korak 2 - dodajemo novi pocetni znak (zasto ovo nije u konstruktoru?)
    grammar.dodajNoviPocetniZnak(GRAMMAR_NEW_BEGIN_STATE);
    
    DEBUG = true;
    if (DEBUG) {
        grammar.dbgPrintFileLines();
        printf("\n");
        grammar.printInfo(); 
    }

    // korak 3 - konstrukcija eNKA iz gramatike
    eNKA enka(grammar);

    // korak 4 - konstrukcija DKA iz eNKA
    DKA dka(enka);

    // korak 5 - konstrukcija tablice parsiranja
    ParsingTable table(dka, grammar); 

    // korak 6 - ispis tablice parsiranja
    table.outputToFile("analizator/tablica2.txt", grammar);
    
    return 0;
}