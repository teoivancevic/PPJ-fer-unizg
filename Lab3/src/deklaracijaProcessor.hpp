#ifndef DEKLARACIJA_PROCESSOR_HPP
#define DEKLARACIJA_PROCESSOR_HPP

#include "utils.hpp"

// Za obradu deklaracija i definicija (4.4.6)
class DeklaracijaProcessor {
public:
    DeklaracijaProcessor(SymbolTable* scope) : currentScope(scope) {}

    // void process_definicija_funkcije(Node* node);
    // void process_lista_parametara(Node* node);
    // void process_deklaracija_parametra(Node* node);
    // void process_lista_deklaracija(Node* node);
    // void process_deklaracija(Node* node);
    // void process_lista_init_deklaratora(Node* node);
    // void process_init_deklarator(Node* node);
    // void process_izravni_deklarator(Node* node);
    // void process_inicijalizator(Node* node);

private:
    SymbolTable* currentScope; 
};




#endif // UTILS_HPP