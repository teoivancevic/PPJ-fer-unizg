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


void FRISCGenerator::DeklaracijaProcessor::process_lista_parametara(Node *node) {
    // <lista_parametara> ::= <deklaracija_parametra>
    // <lista_parametara> ::= <lista_parametara> ZAREZ <deklaracija_parametra>
    
    if (node->children.size() == 1) {
        // Single parameter
        process_deklaracija_parametra(node->children[0]);
        // Propagate values up
        node->evaluatedValue = node->children[0]->evaluatedValue;
        node->evaluatedValues = node->children[0]->evaluatedValues;
        node->evaluatedValueString = node->children[0]->evaluatedValueString;
    } else {
        // Multiple parameters
        process_lista_parametara(node->children[0]);
        process_deklaracija_parametra(node->children[2]);
        
        // Combine and propagate values from both children
        node->evaluatedValues = node->children[0]->evaluatedValues;  // Get existing values
        vector<int> paramValues = node->children[2]->evaluatedValues;  // Get new parameter values
        node->evaluatedValues.insert(node->evaluatedValues.end(), 
                                   paramValues.begin(), paramValues.end());  // Combine
    }
}

void FRISCGenerator::DeklaracijaProcessor::process_deklaracija_parametra(Node *node) {
    // <deklaracija_parametra> ::= <ime_tipa> IDN
    // <deklaracija_parametra> ::= <ime_tipa> IDN L_UGL_ZAGRADA D_UGL_ZAGRADA
    
    string paramName = node->children[1]->lexicalUnit;
    bool isArray = (node->children.size() > 2);
    
    // Store parameter information
    node->evaluatedValueString = paramName;
    if (isArray) {
        node->arraySize = -1;  // Indicates it's an array parameter
    }
    
    // Set evaluated values for potential propagation
    if (isArray) {
        node->evaluatedValues = vector<int>{-1};  // Special value for array parameters
    } else {
        node->evaluatedValues = vector<int>{0};  // Regular parameter
    }
}

void FRISCGenerator::DeklaracijaProcessor::process_lista_deklaracija(Node *node) {
    // <lista_deklaracija> ::= <deklaracija>
    // <lista_deklaracija> ::= <lista_deklaracija> <deklaracija>
    
    if (node->children.size() == 1) {
        // Single declaration
        process_deklaracija(node->children[0]);
        // Propagate values up
        node->evaluatedValue = node->children[0]->evaluatedValue;
        node->evaluatedValues = node->children[0]->evaluatedValues;
        node->evaluatedValueString = node->children[0]->evaluatedValueString;
    } else {
        // Multiple declarations
        process_lista_deklaracija(node->children[0]);
        process_deklaracija(node->children[1]);
        
        // Combine values from both children
        node->evaluatedValues = node->children[0]->evaluatedValues;
        vector<int> newValues = node->children[1]->evaluatedValues;
        node->evaluatedValues.insert(node->evaluatedValues.end(), 
                                   newValues.begin(), newValues.end());
        
        // Keep the latest string value
        node->evaluatedValueString = node->children[1]->evaluatedValueString;
    }
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
    if (node->children.size() == 1) {
        // <init_deklarator> ::= <izravni_deklarator>
        process_izravni_deklarator(node->children[0]);
    }
    else {
        // <init_deklarator> ::= <izravni_deklarator> OP_PRIDRUZI <inicijalizator>
        string varName = node->children[0]->children[0]->lexicalUnit;
        emit("G_" + varName);
        process_inicijalizator(node->children[2], false);
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
    } 
    else {
        // <inicijalizator> ::= L_VIT_ZAGRADA <lista_izraza_pridruzivanja> D_VIT_ZAGRADA
        process_lista_izraza_pridruzivanja(node->children[1]);
    }
}

void FRISCGenerator::DeklaracijaProcessor::process_lista_izraza_pridruzivanja(Node *node) {
    if (node->children.size() == 1) {
        FG->izrazProcessor.process_izraz_pridruzivanja(node->children[0]);
        emit("    DW %D " + to_string(node->children[0]->evaluatedValue));
    } 
    else {
        process_lista_izraza_pridruzivanja(node->children[0]);
        FG->izrazProcessor.process_izraz_pridruzivanja(node->children[2]);
        emit("    DW %D " + to_string(node->children[2]->evaluatedValue));
    }
}