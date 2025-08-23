#include "SemanticAnalyzer.hpp"

using namespace TreeUtils;

// Ovdje je sus //
// <primarni_izraz> ::= IDN | BROJ | NIZ_ZNAKOVA | L_ZAGRADA <izraz> D_ZAGRADA
void SemanticAnalyzer::IzrazProcessor::process_primarni_izraz(Node *node)
{
    cerr << "<primarni_izraz>" << endl;

    Node *child = node->children[0];

    // enclosed expr
    if (node->children.size() == 3)
    {
        process_izraz(node->children[1]);
        child = node->children[1];
    }
    // Identificator
    else if (child->symbol == "IDN")
    {
        SymbolTableEntry *entry = currentScope->lookup(child->lexicalUnit);

        if (!entry)
            return reportError(node);

        child->entry = *entry;
        child->isLValue = child->typeInfo.lValue();
    }
    // Constant
    else if (!Validator::validateConstant(child))
        return reportError(node);

    node->typeInfo = child->typeInfo;
    node->isLValue = child->isLValue;

    // cerr << "!" << node->typeInfo.toString() << "!" << endl;
}

// <postfiks_izraz> ::= <primarni_izraz>
// <postfiks_izraz> ::= <postfiks_izraz> L_UGL_ZAGRADA <izraz> D_UGL_ZAGRADA
// <postfiks_izraz> ::= <postfiks_izraz> L_ZAGRADA D_ZAGRADA
// <postfiks_izraz> ::= <postfiks_izraz> L_ZAGRADA <lista_argumenata> D_ZAGRADA
// <postfiks_izraz> ::= <postfiks_izraz> (OP_INC | OP_DEC)
void SemanticAnalyzer::IzrazProcessor::process_postfiks_izraz(Node *node)
{
    cerr << "<postfiks_izraz>" << endl;

    SA->process(node->children[0]);
    Node *first = node->children[0];

    node->typeInfo = first->typeInfo;
    node->isLValue = first->isLValue;

    if (node->children.size() == 1)
        int debug;

    else if (node->children[1]->symbol == "L_UGL_ZAGRADA")
    {
        if (!first->typeInfo.isArray())
            return reportError(node);

        process_izraz(node->children[2]);
        if (!node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
            return reportError(node);

        node->typeInfo = first->typeInfo.getBaseConstQualified();
        node->isLValue = !first->typeInfo.isConst();
    }
    else if (node->children[1]->symbol == "L_ZAGRADA")
    {
        if (!first->typeInfo.isFunc())
            return reportError(node);

        if (node->children.size() == 3)
        {
            // no args
            if (!first->typeInfo.isVoidParam())
                return reportError(node);
        }
        else
        {
            // check args
            process_lista_argumenata(node->children[2]);

            const auto &params = first->typeInfo.getFunctionParams();
            const auto &args = node->children[2]->typeInfo.getFunctionParams();

            if (params.size() != args.size())
                return reportError(node);

            for (size_t i = 0; i < params.size(); i++)
                if (!args[i].canImplicitlyConvertTo(params[i]))
                    return reportError(node);
        }

        node->typeInfo = TypeInfo(first->typeInfo.getReturnType());
        node->isLValue = false;
    }
    else if (node->children[1]->symbol == "OP_INC" ||
             node->children[1]->symbol == "OP_DEC")
    {
        if (!first->isLValue || !first->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
            return reportError(node);

        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
    }

    cerr << "!" << node->typeInfo.toString() << "!" << endl;
}

// <lista_argumenata> ::= <izraz_pridruzivanja>
// <lista_argumenata> ::= <lista_argumenata> ZAREZ <izraz_pridruzivanja>
void SemanticAnalyzer::IzrazProcessor::process_lista_argumenata(Node *node)
{
    cerr << "<lista_argumenta>" << endl;

    vector<TypeInfo> params;

    if (node->children.size() == 1)
    {
        process_izraz_pridruzivanja(node->children[0]);
        params = {node->children[0]->typeInfo};
    }
    else if (node->children.size() == 3)
    {
        process_lista_argumenata(node->children[0]);
        process_izraz_pridruzivanja(node->children[2]);

        params = node->children[0]->typeInfo.getFunctionParams();
        params.push_back(node->children[2]->typeInfo);
    }

    node->typeInfo = TypeInfo(BasicType::VOID, params);
}

// <unarni_izraz> ::= <postfiks_izraz>
// <unarni_izraz> ::= (OP_INC | OP_DEC) <unarni_izraz>
// <unarni_izraz> ::= <unarni_operator> <cast_izraz>
void SemanticAnalyzer::IzrazProcessor::process_unarni_izraz(Node *node)
{
    cerr << "<unarni_izraz>" << endl;

    if (node->children.size() == 1)
        delegate(node);

    else if (node->children.size() == 2)
    {
        SA->process(node->children[1]);

        if (node->children[0]->isTerminating() && !node->children[1]->isLValue ||
            !node->children[1]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
            return reportError(node);

        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
    }

    cerr << "!" << node->typeInfo.toString() << "!" << endl;
}

//<unarni operator> ::= PLUS | MINUS | OP_TILDA | OP_NEG
void SemanticAnalyzer::IzrazProcessor::process_unarni_operator(Node *node)
{
    cerr << "<unarni_operator>" << endl;
}

// <cast_izraz> ::= <unarni_izraz>
// <cast_izraz> ::= L_ZAGRADA <ime_tipa> D_ZAGRADA <cast_izraz>
void SemanticAnalyzer::IzrazProcessor::process_cast_izraz(Node *node)
{
    cerr << "<cast_izraz>" << endl;

    if (node->children.size() == 1)
        delegate(node);

    else if (node->children.size() == 4)
    {
        process_ime_tipa(node->children[1]);
        process_cast_izraz(node->children[3]);

        TypeInfo targetType = node->children[1]->typeInfo;
        TypeInfo sourceType = node->children[3]->typeInfo;

        if (!sourceType.canExplicitlyConvertTo(targetType))
            return reportError(node);

        node->typeInfo = targetType;
        node->isLValue = false;
    }

    cerr << "!" << node->typeInfo.toString() << "!" << endl;
}

// <ime_tipa> ::= <specifikator_tipa>
// <ime_tipa> ::= KR_CONST <specifikator_tipa>
void SemanticAnalyzer::IzrazProcessor::process_ime_tipa(Node *node)
{
    cerr << "<ime_tipa>" << endl;

    if (node->children.size() == 1)
        delegate(node);

    else if (node->children.size() == 2)
    {
        process_specifikator_tipa(node->children[1]);

        if (node->children[1]->typeInfo.isVoid())
            return reportError(node);

        node->typeInfo = TypeInfo(node->children[1]->typeInfo.getBaseType(), true);
    }
}

// <specifikator_tipa> ::= KR_VOID | KR_CHAR | KR_INT
void SemanticAnalyzer::IzrazProcessor::process_specifikator_tipa(Node *node)
{
    cerr << "<specifikator_tipa>" << endl;

    string typeSpecifier = node->children[0]->symbol;

    if (typeSpecifier == "KR_VOID")
        node->typeInfo = TypeInfo(BasicType::VOID);

    else if (typeSpecifier == "KR_CHAR")
        node->typeInfo = TypeInfo(BasicType::CHAR);

    else if (typeSpecifier == "KR_INT")
        node->typeInfo = TypeInfo(BasicType::INT);
}

// GENERIC EXPRESSION
// <algebarski_izraz> ::= <algebarski_izraz>*
// <algebarski_izraz> ::= <algebarski_izraz> (OP1 | OP2 | ...) <algebarski_izraz>*
void SemanticAnalyzer::IzrazProcessor::process_algebarski_izraz(Node *node)
{
    cerr << "<algebarski_izraz>" << endl;

    if (node->children.size() == 1)
        delegate(node);

    else if (node->children.size() == 3)
    {
        Node *first = node->children[0];

        SA->process(first);

        // cerr << first->symbol << endl;
        // cerr << first->lineNumber << " " << first->typeInfo.toString() << endl;

        if (!node->children[0]->typeInfo.canImplicitlyConvertTo(BasicType::INT))
            return reportError(node);

        cerr << "PRVI" << endl;

        SA->process(node->children[2]);
        if (!node->children[2]->typeInfo.canImplicitlyConvertTo(BasicType::INT))
            return reportError(node);

        cerr << "DRUGI" << endl;

        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
    }

    cerr << "!" << node->symbol << "! " << node->typeInfo.toString() << endl;
}

// <izraz_pridruzivanja> ::= <algebarski_izraz>
// <izraz_pridruzivanja> ::= <postfiks_izraz> OP_PRIDRUZI <izraz_pridruzivanja>
void SemanticAnalyzer::IzrazProcessor::process_izraz_pridruzivanja(Node *node)
{
    cerr << "<izraz_pridruzivanja>" << endl;

    if (node->children.size() == 1)
        delegate(node);

    else if (node->children.size() == 3)
    {
        process_postfiks_izraz(node->children[0]);
        if (!node->children[0]->isLValue)
            return reportError(node);

        process_izraz_pridruzivanja(node->children[2]);

        // cerr << "YESSS " << node->children[0]->typeInfo.isArray() << " " << node->children[2]->typeInfo.isArray() << endl;
        if (!(node->children[2]->typeInfo.canImplicitlyConvertTo(node->children[0]->typeInfo)))
            return reportError(node);
        // cout << "BYE" << endl;

        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = false;
    }
}

// <izraz> ::= <izraz_pridruzivanja>
// <izraz> ::= <izraz> ZAREZ <izraz_pridruzivanja>
void SemanticAnalyzer::IzrazProcessor::process_izraz(Node *node)
{
    cerr << "<izraz>" << endl;

    if (node->children.size() == 1)
    {
        process_izraz_pridruzivanja(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
    }
    else if (node->children.size() == 3)
    {
        process_izraz(node->children[0]);
        process_izraz_pridruzivanja(node->children[2]);
        node->typeInfo = node->children[2]->typeInfo; // Type of the last expression
        node->isLValue = false;
    }
}

void SemanticAnalyzer::IzrazProcessor::process_multiplikativni_izraz(Node *node) { return process_algebarski_izraz(node); }

void SemanticAnalyzer::IzrazProcessor::process_aditivni_izraz(Node *node) { return process_algebarski_izraz(node); }

void SemanticAnalyzer::IzrazProcessor::process_odnosni_izraz(Node *node) { return process_algebarski_izraz(node); }

void SemanticAnalyzer::IzrazProcessor::process_jednakosni_izraz(Node *node) { return process_algebarski_izraz(node); }

void SemanticAnalyzer::IzrazProcessor::process_bin_i_izraz(Node *node) { return process_algebarski_izraz(node); }

void SemanticAnalyzer::IzrazProcessor::process_bin_xili_izraz(Node *node) { return process_algebarski_izraz(node); }

void SemanticAnalyzer::IzrazProcessor::process_bin_ili_izraz(Node *node) { return process_algebarski_izraz(node); }

void SemanticAnalyzer::IzrazProcessor::process_log_i_izraz(Node *node) { return process_algebarski_izraz(node); }

void SemanticAnalyzer::IzrazProcessor::process_log_ili_izraz(Node *node) { return process_algebarski_izraz(node); }