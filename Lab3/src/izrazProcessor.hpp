#ifndef IZRAZ_PROCESSOR_HPP
#define IZRAZ_PROCESSOR_HPP


#include "utils.hpp"


// Za obradu izraza (4.4.4)
class IzrazProcessor {
public:
    IzrazProcessor(SymbolTable* scope) : currentScope(scope) {}

    void process_primarni_izraz(Node* node);
    // void process_postfiks_izraz(Node* node);
    // void process_lista_argumenata(Node* node);
    // void process_unarni_izraz(Node* node);
    // void process_unarni_operator(Node* node);
    // void process_cast_izraz(Node* node);
    // void process_ime_tipa(Node* node);
    // void process_specifikator_tipa(Node* node);
    // void process_multiplikativni_izraz(Node* node);
    // void process_aditivni_izraz(Node* node);
    // void process_odnosni_izraz(Node* node);
    // void process_jednakosni_izraz(Node* node);
    // void process_bin_i_izraz(Node* node);
    // void process_bin_xili_izraz(Node* node);
    // void process_bin_ili_izraz(Node* node);
    // void process_log_i_izraz(Node* node);
    // void process_log_ili_izraz(Node* node);
    void process_izraz_pridruzivanja(Node* node); // not implemented
    void process_izraz(Node* node);

private:
    SymbolTable* currentScope;  // Add this member variable
};

// Implementation of the public function
void IzrazProcessor::process_primarni_izraz(Node* node) {
    if (node->children.size() != 1 && node->children.size() != 3) {
        reportError(node);
        return;
    }

    if (node->children.size() == 3) {
        if (node->children[0]->content.find("L_ZAGRADA") == 0 &&
            node->children[2]->content.find("D_ZAGRADA") == 0) {
            process_izraz(node->children[1]);
            node->typeInfo = node->children[1]->typeInfo;
            node->isLValue = node->children[1]->isLValue;
            return;
        }
        reportError(node);
        return;
    }

    Node* child = node->children[0];
    string childContent = child->content;
    node->isLValue = false;

    if (childContent.find("IDN") == 0) {
        SymbolTableEntry* entry = currentScope->lookup(child->lexicalUnit);
        // if (!entry) {
        //     cout << "funkcija" << endl;  // Function not found at all
        //     exit(0);
        // }

        if (!entry) {
            reportError(node);  // Function not found at all
            return;
        }
        
        // If it's a function and it's not defined (only declared)
        // if (entry->isFunction && !entry->isDefined) {
        //     cout << "funkcija" << endl;
        //     exit(0);
        // }
        
        

        
        
        if (entry->type == "int") {
            node->typeInfo = TypeInfo(BasicType::INT, entry->isConstant);
        } else if (entry->type == "char") {
            node->typeInfo = TypeInfo(BasicType::CHAR, entry->isConstant);
        } else if (entry->type == "void") {
            node->typeInfo = TypeInfo(BasicType::VOID, entry->isConstant);
        }
        
        node->isLValue = !entry->isConstant && !entry->isFunction && 
                        entry->arraySize == -1 && 
                        (entry->type == "int" || entry->type == "char");
    }
    else if (childContent.find("BROJ") == 0) {
        if (!ExpressionValidator::validateBrojConstant(child->lexicalUnit, node->typeInfo)) {
            reportError(node);
        }
    }
    else if (childContent.find("ZNAK") == 0) {
        if (!ExpressionValidator::validateZnakConstant(child->lexicalUnit, node->typeInfo)) {
            reportError(node);
        }
    }
    else if (childContent.find("NIZ_ZNAKOVA") == 0) {
        if (!ExpressionValidator::validateNizZnakova(child->lexicalUnit, node->typeInfo)) {
            reportError(node);
        }
    }
    else {
        reportError(node);
    }
}

void IzrazProcessor::process_izraz_pridruzivanja(Node* node){
    // currently not implemented
}

void IzrazProcessor::process_izraz(Node* node) {
    if (!node) {
        reportError(node);
        return;
    }
    
    // According to 4.4.4, <izraz> can be either:
    // <izraz> ::= <izraz_pridruzivanja>
    // <izraz> ::= <izraz> ZAREZ <izraz_pridruzivanja>
    
    if (node->children.size() == 1) {
        // <izraz> ::= <izraz_pridruzivanja>
        process_izraz_pridruzivanja(node->children[0]);
        node->type = node->children[0]->type;
        node->isLValue = false;  // Comma expressions are never l-values
    }
    else if (node->children.size() == 3) {
        // <izraz> ::= <izraz> ZAREZ <izraz_pridruzivanja>
        process_izraz(node->children[0]);
        process_izraz_pridruzivanja(node->children[2]);
        node->type = node->children[2]->type;  // Type of the last expression
        node->isLValue = false;
    }
    else {
        reportError(node);
    }
}



#endif // UTILS_HPP