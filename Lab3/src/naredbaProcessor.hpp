#ifndef NAREDBA_PROCESSOR_HPP
#define NAREDBA_PROCESSOR_HPP


#include "utils.hpp"

// Za obradu programske strukture (4.4.5)
class NaredbaProcessor {
public:
    NaredbaProcessor(SymbolTable* scope) : currentScope(scope) {}

    // void process_slozena_naredba(Node* node);
    // void process_lista_naredbi(Node* node);
    // void process_naredba(Node* node);
    // void process_izraz_naredba(Node* node);
    // void process_naredba_grananja(Node* node);
    // void process_naredba_petlje(Node* node);
    void process_naredba_skoka(Node* node);
    // void process_prijevodna_jedinica(Node* node);
    // void process_vanjska_deklaracija(Node* node);

private:
    SymbolTable* currentScope; 
};



void NaredbaProcessor::process_naredba_skoka(Node* node){
    // not implemented
}



#endif // UTILS_HPP