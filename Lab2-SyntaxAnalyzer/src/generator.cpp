#include <iostream>
#include <fstream>
#include "utils.hpp"
#include "automat.hpp"
#include "grammar.hpp"

struct Action 
{
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
                //provjera vrijedi li pravilo a) --//-- ... uz dodatni uvjet prednosti iz uputa labosa
                if (dka.exists_trans(current, sym)) 
                {
                    State nextState = dka.transitions.at(current).at(sym);
                    
                    if (grammar.isTerminating(sym))
                        akcija.emplace(key, Action{"POMAKNI", nextState});
                    else 
                        novoStanje.emplace(key, Action{"STAVI", nextState});
                }   
                //provjerava vrijedi li pravilo b) --//--
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


std::string input = "cin";


int main ()
{
    // korak 1 - parsiranje gramatike

    // Grammar grammar(input);
    Grammar grammar("cin");
    
    // korak 2 - dodajemo novi pocetni znak (zasto ovo nije u konstruktoru?)
    grammar.dodajNoviPocetniZnak(GRAMMAR_NEW_BEGIN_STATE);


    // korak 3 - konstrukcija eNKA iz gramatike
    eNKA enka(grammar);

    // korak 4 - konstrukcija DKA iz eNKA
    DKA dka(enka);

    // korak 5 - konstrukcija tablice parsiranja
    ParsingTable table(dka, grammar); 

    // korak 6 - ispis tablice parsiranja
    table.outputToFile("analizator/tablica.txt", grammar);
    
    return 0;
}