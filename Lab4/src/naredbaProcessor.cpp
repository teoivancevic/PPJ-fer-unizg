#include "FRISCGenerator.hpp"

using namespace TreeUtils;

// <prijevodna_jedinica> ::= <vanjska_deklaracija>
// <prijevodna_jedinica> ::= <prijevodna_jedinica> <vanjska_deklaracija>
void FRISCGenerator::NaredbaProcessor::process_prijevodna_jedinica(Node *node, bool isFunctionPass) { 
    if (node->children.size() == 1) {
        process_vanjska_deklaracija(node->children[0], isFunctionPass);
    }
    else {
        process_prijevodna_jedinica(node->children[0], isFunctionPass);
        process_vanjska_deklaracija(node->children[1], isFunctionPass);
    }
}

// <vanjska_deklaracija> ::= <definicija_funkcije> | <deklaracija>
void FRISCGenerator::NaredbaProcessor::process_vanjska_deklaracija(Node *node, bool isFunctionPass) { 
    if (isFunctionPass && node->children[0]->symbol == "<definicija_funkcije>") {
        FG->deklaracijaProcessor.process_definicija_funkcije(node->children[0]);
    } 
    else if (!isFunctionPass && node->children[0]->symbol == "<deklaracija>") {
        FG->deklaracijaProcessor.process_deklaracija(node->children[0]);
    }
}

// <slozena_naredba> ::= L_VIT_ZAGRADA <lista_naredbi> D_VIT_ZAGRADA
// <slozena_naredba> ::= L_VIT_ZAGRADA <lista_deklaracija> <lista_naredbi> D_VIT_ZAGRADA
void FRISCGenerator::NaredbaProcessor::process_slozena_naredba(Node *node) { 
    // Handle both forms:
    // <slozena_naredba> ::= L_VIT_ZAGRADA <lista_naredbi> D_VIT_ZAGRADA
    // <slozena_naredba> ::= L_VIT_ZAGRADA <lista_deklaracija> <lista_naredbi> D_VIT_ZAGRADA
    for (Node* child : node->children) {
        if (child->symbol == "<lista_deklaracija>") {
            FG->deklaracijaProcessor.process_lista_deklaracija(child);
        }
        else if (child->symbol == "<lista_naredbi>") {
            process_lista_naredbi(child);
        }
    }
}

// <lista_naredbi> ::= <naredba>
// <lista_naredbi> ::= <lista_naredbi> <naredba>
void FRISCGenerator::NaredbaProcessor::process_lista_naredbi(Node *node) { 
    if (node->children.size() == 1) {
        process_naredba(node->children[0]);
    }
    else {
        process_lista_naredbi(node->children[0]);
        process_naredba(node->children[1]);
    }

}

// <process_naredba> ::= <izraz_naredba> | <naredba_grananja> | <naredba_petlje> | <naredba_skoka> | <slozena_naredba>
void FRISCGenerator::NaredbaProcessor::process_naredba(Node *node) { 
    if (node->children[0]->symbol == "<izraz_naredba>") {
        process_izraz_naredba(node->children[0]);
    }
    else if (node->children[0]->symbol == "<naredba_grananja>") {
        process_naredba_grananja(node->children[0]);
    }
    else if (node->children[0]->symbol == "<naredba_petlje>") {
        process_naredba_petlje(node->children[0]);
    }
    else if (node->children[0]->symbol == "<naredba_skoka>") {
        process_naredba_skoka(node->children[0]);
    }
    else if (node->children[0]->symbol == "<slozena_naredba>") {
        process_slozena_naredba(node->children[0]);
    }
}

// <naredba_grananja> ::= KR_IF L_ZAGRADA <izraz> D_ZAGRADA <naredba>
    // <naredba_grananja> ::= KR_IF L_ZAGRADA <izraz> D_ZAGRADA <naredba> KR_ELSE <naredba>
void FRISCGenerator::NaredbaProcessor::process_naredba_grananja(Node *node)
{
    int currentLabelId = FG->labelCounter++;
    
    // Process the condition expression
    FG->izrazProcessor.process_izraz(node->children[2]);
    
    if (node->children.size() == 5) {
        // If statement without else
        string labelEnd = "END_IF_" + to_string(currentLabelId);
        
        // If condition is false (0), jump to end
        emit("    CMP R6, %D 0");
        emit("    JP_EQ " + labelEnd);
        
        // Process the true branch
        process_naredba(node->children[4]);
        
        // End label
        emit(labelEnd);
    }
    else if (node->children.size() == 7) {
        // If statement with else
        string labelElse = "ELSE_" + to_string(currentLabelId);
        string labelEnd = "END_IF_" + to_string(currentLabelId);
        
        // If condition is false (0), jump to else
        emit("    CMP R6, %D 0");
        emit("    JP_EQ " + labelElse);
        
        // Process the true branch
        process_naredba(node->children[4]);
        emit("    JP " + labelEnd);
        
        // Else branch
        emit(labelElse);
        process_naredba(node->children[6]);
        
        // End label
        emit(labelEnd);
    }
}

// <naredba_petlje> ::= KR_WHILE L_ZAGRADA <izraz> D_ZAGRADA <naredba>
// <naredba_petlje> ::= KR_FOR L_ZAGRADA <izraz_naredba> <izraz_naredba> D_ZAGRADA <naredba>
// <naredba_petlje> ::= KR_FOR L_ZAGRADA <izraz_naredba> <izraz_naredba> <izraz> D_ZAGRADA <naredba>
void FRISCGenerator::NaredbaProcessor::process_naredba_petlje(Node *node) {
    int currentLabel = FG->labelCounter++;
    
    // WHILE loop
    if (node->children[0]->content.find("KR_WHILE") == 0) {
        string startLabel = "WHILE_" + to_string(currentLabel);
        string endLabel = "WHILE_END_" + to_string(currentLabel);
        
        // Loop start label
        emit(startLabel);
        
        // Process condition
        FG->izrazProcessor.process_izraz(node->children[2]);
        
        // If condition is false (0), jump to end
        emit("    CMP R6, %D 0");
        emit("    JP_EQ " + endLabel);
        
        // Process loop body
        process_naredba(node->children[4]);
        
        // Jump back to start
        emit("    JP " + startLabel);
        
        // End label
        emit(endLabel);
    }
    // FOR loop
    else if (node->children[0]->content.find("KR_FOR") == 0) {
        string startLabel = "FOR_" + to_string(currentLabel);
        string endLabel = "FOR_END_" + to_string(currentLabel);
        
        // Process initialization
        process_izraz_naredba(node->children[2]);
        
        // Loop start label
        emit(startLabel);
        
        // Process condition
        process_izraz_naredba(node->children[3]);
        emit("    CMP R6, %D 0");
        emit("    JP_EQ " + endLabel);
        
        // Process loop body
        process_naredba(node->children[node->children.size() - 1]);
        
        // Process increment if it exists
        if (node->children.size() == 7) {
            FG->izrazProcessor.process_izraz(node->children[4]);
        }
        
        // Jump back to condition check
        emit("    JP " + startLabel);
        
        // End label
        emit(endLabel);
    }
}

// <naredba_skoka> ::= KR_BREAK | KR_CONTINUE
// <naredba_skoka> ::= KR_RETURN TOCKAZAREZ
// <naredba_skoka> ::= KR_RETURN <izraz> TOCKAZAREZ
void FRISCGenerator::NaredbaProcessor::process_naredba_skoka(Node *node)
{
    // <naredba_skoka> ::= KR_RETURN TOCKAZAREZ
    // <naredba_skoka> ::= KR_RETURN <izraz> TOCKAZAREZ
    if (node->children[0]->content.find("KR_RETURN") == 0) {
        if (node->children.size() == 3) { // Has expression
            FG->izrazProcessor.process_izraz(node->children[1]);
        }
        
        // Specifically for function calls, we want to add a special handling
        emit("    RET");
    }
}

// <izraz_naredba> ::= TOCKAZAREZ
// <izraz_naredba> ::= <izraz> TOCKAZAREZ
void FRISCGenerator::NaredbaProcessor::process_izraz_naredba(Node *node) {
    // <izraz_naredba> ::= TOCKAZAREZ
    if (node->children.size() == 1) {
        // Empty expression statement - nothing to do
        node->evaluatedValue = 1;  // Default value for empty condition (for loops)
    }
    // <izraz_naredba> ::= <izraz> TOCKAZAREZ
    else if (node->children.size() == 2) {
        FG->izrazProcessor.process_izraz(node->children[0]);
        // Propagate the expression's value up
        node->evaluatedValue = node->children[0]->evaluatedValue;
        node->evaluatedValueString = node->children[0]->evaluatedValueString;
    }
}

