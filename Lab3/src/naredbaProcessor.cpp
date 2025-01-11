#include "SemanticAnalyzer.hpp"

using namespace TreeUtils;

// <prijevodna_jedinica> ::= <vanjska_deklaracija>
// <prijevodna_jedinica> ::= <prijevodna_jedinica> <vanjska_deklaracija>
void SemanticAnalyzer::NaredbaProcessor::process_prijevodna_jedinica(Node *node) { processAll(node); }

// <vanjska_deklaracija> ::= <definicija_funkcije> | <deklaracija>
void SemanticAnalyzer::NaredbaProcessor::process_vanjska_deklaracija(Node *node) { processAll(node); }

// <slozena_naredba> ::= L_VIT_ZAGRADA <lista_naredbi> D_VIT_ZAGRADA
// <slozena_naredba> ::= L_VIT_ZAGRADA <lista_deklaracija> <lista_naredbi> D_VIT_ZAGRADA
void SemanticAnalyzer::NaredbaProcessor::process_slozena_naredba(Node *node) { processAll(node); }

// <lista_naredbi> ::= <naredba>
// <lista_naredbi> ::= <lista_naredbi> <naredba>
void SemanticAnalyzer::NaredbaProcessor::process_lista_naredbi(Node *node) { processAll(node); }

// <process_naredba> ::= <izraz_naredba> | <naredba_grananja> | <naredba_petlje> | <naredba_skoka> | <slozena_naredba>
void SemanticAnalyzer::NaredbaProcessor::process_naredba(Node *node) { processAll(node); }

// <naredba_grananja> ::= KR_IF L_ZAGRADA <izraz> D_ZAGRADA <naredba>
// <naredba_grananja> ::= KR_IF L_ZAGRADA <izraz> D_ZAGRADA <naredba> KR_ELSE <naredba>
void SemanticAnalyzer::NaredbaProcessor::process_naredba_grananja(Node *node)
{
    // log expr must be int convertible
    SA->izrazProcessor.process_izraz(node->children[2]);
    if (!node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        return reportError(node);

    processAll(node);
}

// <naredba_petlje> ::= KR_WHILE L_ZAGRADA <izraz> D_ZAGRADA <naredba>
// <naredba_petlje> ::= KR_FOR L_ZAGRADA <izraz_naredba> <izraz_naredba> D_ZAGRADA <naredba>
// <naredba_petlje> ::= KR_FOR L_ZAGRADA <izraz_naredba> <izraz_naredba> <izraz> D_ZAGRADA <naredba>
void SemanticAnalyzer::NaredbaProcessor::process_naredba_petlje(Node *node)
{
    if (node->children.size() == 5)
    {
        SA->izrazProcessor.process_izraz(node->children[2]);
        if (!node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
            return reportError(node);

        process_naredba(node->children[4]);
    }
    else if (node->children.size() == 6 || node->children.size() == 7)
    {
        process_izraz_naredba(node->children[2]);
        process_izraz_naredba(node->children[3]);
        if (!node->children[3]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
            return reportError(node);

        if (node->children.size() == 7)
            SA->izrazProcessor.process_izraz(node->children[4]);

        process_naredba(node->children[5]);
    }
}

// <naredba_skoka> ::= KR_BREAK | KR_CONTINUE
// <naredba_skoka> ::= KR_RETURN TOCKAZAREZ
// <naredba_skoka> ::= KR_RETURN <izraz> TOCKAZAREZ
void SemanticAnalyzer::NaredbaProcessor::process_naredba_skoka(Node *node)
{
    if (node->children[0]->content.find("KR_CONTINUE") == 0 ||
        node->children[0]->content.find("KR_BREAK") == 0)
    {
        // Check if inside a loop
        bool insideLoop = false;
        for (Node *current = node; current->parent != nullptr; current = current->parent)
        {
            if (current->symbol == "<naredba_petlje>")
            {
                insideLoop = true;
                break;
            }
        }
        if (!insideLoop)
            return reportError(node);
    }
    else if (node->children[0]->content.find("KR_RETURN") == 0)
    {
        // Find enclosing function
        Node *current = node;
        while (current && current->symbol != "<definicija_funkcije>")
            current = current->parent;

        if (!current)
            return reportError(node);

        TypeInfo returnType; // VOID
        if (node->children.size() == 3)
        {
            // Process the return expression
            SA->izrazProcessor.process_izraz(node->children[1]);
            returnType = node->children[1]->typeInfo;
        }

        // check returnType compatibility
        if (!returnType.canImplicitlyConvertTo(current->typeInfo.getReturnType()))
            return reportError(node);
    }
}

// <izraz_naredba> ::= TOCKAZAREZ
// <izraz_naredba> ::= <izraz> TOCKAZAREZ
void SemanticAnalyzer::NaredbaProcessor::process_izraz_naredba(Node *node)
{
    node->typeInfo = TypeInfo(BasicType::INT); // Default type for empty expression

    if (node->children.size() == 2)
    {
        SA->izrazProcessor.process_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
    }
}
