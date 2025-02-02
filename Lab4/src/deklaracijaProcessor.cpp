#include "FRISCGenerator.hpp"

using namespace TreeUtils;
using namespace TypeUtils;

void FRISCGenerator::DeklaracijaProcessor::process_definicija_funkcije(Node *node)
{
    // Get function name and emit label
    string funcName = node->children[1]->lexicalUnit;
    // Convert function name to uppercase for FRISC convention
    transform(funcName.begin(), funcName.end(), funcName.begin(), ::toupper);
    emit("F_" + funcName);

    // Process function body
    for (Node* child : node->children) {
        if (child->symbol == "<slozena_naredba>") {
            FG->naredbaProcessor.process_slozena_naredba(child);
            break;
        }
    }
}

void FRISCGenerator::DeklaracijaProcessor::process_lista_parametara(Node *node)
{
    
}

void FRISCGenerator::DeklaracijaProcessor::process_deklaracija_parametra(Node *node)
{
    
}


void FRISCGenerator::DeklaracijaProcessor::process_lista_deklaracija(Node *node)
{
    
}

void FRISCGenerator::DeklaracijaProcessor::process_deklaracija(Node *node)
{
    // <deklaracija> ::= <ime_tipa> <lista_init_deklaratora> TOCKAZAREZ
    process_lista_init_deklaratora(node->children[1]);
}

void FRISCGenerator::DeklaracijaProcessor::process_lista_init_deklaratora(Node *node)
{
    if (node->children.size() == 1) {
        // <lista_init_deklaratora> ::= <init_deklarator>
        process_init_deklarator(node->children[0]);
    }
    else {
        // <lista_init_deklaratora> ::= <lista_init_deklaratora> ZAREZ <init_deklarator>
        process_lista_init_deklaratora(node->children[0]);
        process_init_deklarator(node->children[2]);
    }
}

void FRISCGenerator::DeklaracijaProcessor::process_init_deklarator(Node *node) {
    string varName = node->children[0]->children[0]->lexicalUnit;
    
    if (node->children.size() > 1) {
        // Process initialization without emitting code
        process_inicijalizator(node->children[2], false);
        int value = node->children[2]->evaluatedValue;
        emit("G_" + varName + " DW %D " + to_string(value));
    } else {
        emit("G_" + varName + " DW %D 0");
    }
}



void FRISCGenerator::DeklaracijaProcessor::process_izravni_deklarator(Node *node) {
    string varName = node->children[0]->lexicalUnit;
    
    if (node->children.size() == 1) {
        // <izravni_deklarator> ::= IDN
        emit("G_" + varName + " DW %D 0");
    }
    else if (node->children[1]->content.find("L_UGL_ZAGRADA") == 0) {
        // <izravni_deklarator> ::= IDN L_UGL_ZAGRADA BROJ D_UGL_ZAGRADA
        string size = node->children[2]->lexicalUnit;
        emit("G_" + varName + " DS %D " + size);
    }
    else if (node->children[1]->content.find("L_ZAGRADA") == 0) {
        // Function declarations - no code generation needed for declarations
        // Actual function definitions are handled elsewhere
    }
}

void FRISCGenerator::DeklaracijaProcessor::process_inicijalizator(Node *node, bool emitCode) {

    if (node->children.size() == 1) {
        // <inicijalizator> ::= <izraz_pridruzivanja>
        FG->izrazProcessor.process_izraz_pridruzivanja(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
    }
    else {
        // <inicijalizator> ::= L_VIT_ZAGRADA <lista_izraza_pridruzivanja> D_VIT_ZAGRADA
        process_lista_izraza_pridruzivanja(node->children[1]);
    }
}


void FRISCGenerator::DeklaracijaProcessor::process_lista_izraza_pridruzivanja(Node *node) {
    if (node->children.size() == 1) {
        // <lista_izraza_pridruzivanja> ::= <izraz_pridruzivanja>
        FG->izrazProcessor.process_izraz_pridruzivanja(node->children[0]);
    }
    else {
        // <lista_izraza_pridruzivanja> ::= <lista_izraza_pridruzivanja> ZAREZ <izraz_pridruzivanja>
        process_lista_izraza_pridruzivanja(node->children[0]);
        FG->izrazProcessor.process_izraz_pridruzivanja(node->children[2]);
    }
}