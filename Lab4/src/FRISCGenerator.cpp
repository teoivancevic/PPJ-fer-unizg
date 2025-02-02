#include "FRISCGenerator.hpp"

void FRISCGenerator::generateCode(Node* node) {
    // Process functions first
    generateFunctions(node);
    
    // Then process declarations
    generateDeclarations(node);
}

void FRISCGenerator::generateFunctions(Node* node) {
    if (node->symbol == "<vanjska_deklaracija>") {
        if (node->children[0]->symbol == "<definicija_funkcije>") {
            deklaracijaProcessor.process_definicija_funkcije(node->children[0]);
        }
    }
    
    // Traverse children
    if (!node->isTerminating()) {
        for (Node* child : node->children) {
            generateFunctions(child);
        }
    }
}

void FRISCGenerator::generateDeclarations(Node* node) {
    if (node->symbol == "<vanjska_deklaracija>") {
        if (node->children[0]->symbol == "<deklaracija>") {
            deklaracijaProcessor.process_deklaracija(node->children[0]);
        }
    }
    
    // Traverse children
    if (!node->isTerminating()) {
        for (Node* child : node->children) {
            generateDeclarations(child);
        }
    }
}