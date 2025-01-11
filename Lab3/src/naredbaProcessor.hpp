#ifndef NAREDBA_PROCESSOR_HPP
#define NAREDBA_PROCESSOR_HPP


#include "utils.hpp"

// Za obradu programske strukture (4.4.5)
class NaredbaProcessor {
public:
    NaredbaProcessor(SymbolTable* scope) : currentScope(scope) {}

    void process_slozena_naredba(Node* node);
    void process_lista_naredbi(Node* node);
    void process_naredba(Node* node);
    void process_izraz_naredba(Node* node);
    void process_naredba_grananja(Node* node);
    void process_naredba_petlje(Node* node);
    void process_naredba_skoka(Node* node);
    void process_prijevodna_jedinica(Node* node);
    void process_vanjska_deklaracija(Node* node);

private:
    SymbolTable* currentScope; 
};

void NaredbaProcessor::process_slozena_naredba(Node* node) {
    // not implemented
}

/*
void NaredbaProcessor::process_slozena_naredba(Node* node) {
    if (!node || node->children.size() < 2) {
        reportError(node);
        return;
    }

    // <slozena_naredba> ::= L_VIT_ZAGRADA <lista_naredbi> D_VIT_ZAGRADA
    if (node->children.size() == 3) {
        process_lista_naredbi(node->children[1]);
        return;
    }

    // <slozena_naredba> ::= L_VIT_ZAGRADA <lista_deklaracija> <lista_naredbi> D_VIT_ZAGRADA
    if (node->children.size() == 4) {
        DeklaracijaProcessor deklaracijaProcessor(currentScope);
        deklaracijaProcessor.process_lista_deklaracija(node->children[1]);
        process_lista_naredbi(node->children[2]);
        return;
    }

    reportError(node);
}
*/

void NaredbaProcessor::process_lista_naredbi(Node* node) {
    if (!node) {
        reportError(node);
        return;
    }

    // <lista_naredbi> ::= <naredba>
    if (node->children.size() == 1) {
        process_naredba(node->children[0]);
        return;
    }

    // <lista_naredbi> ::= <lista_naredbi> <naredba>
    if (node->children.size() == 2) {
        process_lista_naredbi(node->children[0]);
        process_naredba(node->children[1]);
        return;
    }

    reportError(node);
}

void NaredbaProcessor::process_naredba(Node* node) {
    if (!node || node->children.size() != 1) {
        reportError(node);
        return;
    }

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
    else {
        reportError(node);
    }
}


void NaredbaProcessor::process_naredba_grananja(Node* node) {
    if (!node) {
        reportError(node);
        return;
    }

    IzrazProcessor izrazProcessor(currentScope);

    // <naredba_grananja> ::= KR_IF L_ZAGRADA <izraz> D_ZAGRADA <naredba>
    if (node->children.size() == 5) {
        izrazProcessor.process_izraz(node->children[2]);
        if (!node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT))) {
            reportError(node);
            return;
        }
        process_naredba(node->children[4]);
        return;
    }

    // <naredba_grananja> ::= KR_IF L_ZAGRADA <izraz> D_ZAGRADA <naredba> KR_ELSE <naredba>
    if (node->children.size() == 7) {
        izrazProcessor.process_izraz(node->children[2]);
        if (!node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT))) {
            reportError(node);
            return;
        }
        process_naredba(node->children[4]);
        process_naredba(node->children[6]);
        return;
    }

    reportError(node);
}

void NaredbaProcessor::process_naredba_petlje(Node* node) {
    if (!node) {
        reportError(node);
        return;
    }

    IzrazProcessor izrazProcessor(currentScope);

    // <naredba_petlje> ::= KR_WHILE L_ZAGRADA <izraz> D_ZAGRADA <naredba>
    if (node->children.size() == 5 && node->children[0]->content.find("KR_WHILE") == 0) {
        izrazProcessor.process_izraz(node->children[2]);
        if (!node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT))) {
            reportError(node);
            return;
        }
        process_naredba(node->children[4]);
        return;
    }

    // For loops
    if (node->children[0]->content.find("KR_FOR") == 0) {
        // <naredba_petlje> ::= KR_FOR L_ZAGRADA <izraz_naredba> <izraz_naredba> D_ZAGRADA <naredba>
        if (node->children.size() == 6) {
            process_izraz_naredba(node->children[2]);
            process_izraz_naredba(node->children[3]);
            if (!node->children[3]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT))) {
                reportError(node);
                return;
            }
            process_naredba(node->children[5]);
            return;
        }

        // <naredba_petlje> ::= KR_FOR L_ZAGRADA <izraz_naredba> <izraz_naredba> <izraz> D_ZAGRADA <naredba>
        if (node->children.size() == 7) {
            process_izraz_naredba(node->children[2]);
            process_izraz_naredba(node->children[3]);
            if (!node->children[3]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT))) {
                reportError(node);
                return;
            }
            izrazProcessor.process_izraz(node->children[4]);
            process_naredba(node->children[6]);
            return;
        }
    }

    reportError(node);
}

void NaredbaProcessor::process_naredba_skoka(Node* node) {
    if (!node) {
        reportError(node);
        return;
    }

    // Break and continue
    if (node->children.size() == 2 &&
        (node->children[0]->content.find("KR_CONTINUE") == 0 ||
         node->children[0]->content.find("KR_BREAK") == 0)) {
        
        // Check if inside a loop
        bool insideLoop = false;
        Node* current = node;
        while (current && current->symbol != "<prijevodna_jedinica>") {
            if (current->symbol == "<naredba_petlje>") {
                insideLoop = true;
                break;
            }
            current = current->parent;
        }

        if (!insideLoop) {
            reportError(node);
            return;
        }
        return;
    }

    // <naredba_skoka> ::= KR_RETURN TOCKAZAREZ
    if (node->children.size() == 2 && node->children[0]->content.find("KR_RETURN") == 0) {
        // Find enclosing function
        Node* current = node;
        while (current && current->symbol != "<definicija_funkcije>") {
            current = current->parent;
        }

        if (!current) {
            reportError(node);
            return;
        }

        if (!current->typeInfo.isVoid()) {
            // Non-void function must return a value
            reportError(node);//, "<naredba_skoka> ::= KR_RETURN TOCKAZAREZ");
            return;
        }
        return;
    }

    // <naredba_skoka> ::= KR_RETURN <izraz> TOCKAZAREZ
    if (node->children.size() == 3 && node->children[0]->content.find("KR_RETURN") == 0) {
        // Find enclosing function
        Node* current = node;
        while (current && current->symbol != "<definicija_funkcije>") {
            current = current->parent;
        }

        if (!current) {
            reportError(node);
            return;
        }

        // Process the return expression
        IzrazProcessor izrazProcessor(currentScope);
        izrazProcessor.process_izraz(node->children[1]);

        if (current->typeInfo.isVoid()) {
            // Void function must not return a value
            reportError(node);//, "<naredba_skoka> ::= KR_RETURN <izraz> TOCKAZAREZ");
            return;
        }

        if (!node->children[1]->typeInfo.canImplicitlyConvertTo(current->typeInfo)) {
            // Type mismatch in return value
            reportError(node);//, "<naredba_skoka> ::= KR_RETURN <izraz> TOCKAZAREZ");
            return;
        }
        return;
    }

    reportError(node);
}



void NaredbaProcessor::process_izraz_naredba(Node* node) {
    if (!node) {
        reportError(node);
        return;
    }

    // <izraz_naredba> ::= TOCKAZAREZ
    if (node->children.size() == 1) {
        node->typeInfo = TypeInfo(BasicType::INT); // Default type for empty expression
        return;
    }

    // <izraz_naredba> ::= <izraz> TOCKAZAREZ
    if (node->children.size() == 2) {
        IzrazProcessor izrazProcessor(currentScope);
        izrazProcessor.process_izraz(node->children[0]);
        
        // If this is a condition (like in a for loop), the expression must be implicitly
        // convertible to int and must not be a function type
        if (TypeUtils::isFunctionType(node->children[0]->typeInfo)) {
            reportError(node);
            return;
        }

        if (!node->children[0]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT))) {
            reportError(node);
            return;
        }

        node->typeInfo = node->children[0]->typeInfo;
        return;
    }

    reportError(node);
}


void NaredbaProcessor::process_prijevodna_jedinica(Node* node) {
    // Add implementation according to your needs
}

void NaredbaProcessor::process_vanjska_deklaracija(Node* node) {
    // Add implementation according to your needs
}


#endif // UTILS_HPP