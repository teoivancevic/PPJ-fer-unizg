#include "SemanticAnalyzer.hpp"

using namespace TreeUtils;

//<definicija_funkcije> ::= <ime_tipa> IDN L_ZAGRADA KR_VOID D_ZAGRADA <slozena_naredba>
//<definicija_funkcije> ::= <ime_tipa> IDN L_ZAGRADA <lista_parametara> D_ZAGRADA <slozena_naredba>
void SemanticAnalyzer::DeklaracijaProcessor::process_definicija_funkcije(Node *node)
{
    cerr << "<definicija_funkcije>" << endl;

    Node *type_name = node->children[0];
    Node *list = node->children[3];
    Node *body = node->children[5];

    SA->izrazProcessor.process_ime_tipa(type_name);

    TypeInfo returnType = type_name->typeInfo;
    node->name = node->children[1]->lexicalUnit;

    SymbolTableEntry *existingFunc = currentScope->lookup(node->name);

    if (returnType.isConst() || (existingFunc && (existingFunc->isDefined)))
        return reportError(node);

    vector<TypeInfo> params;

    if (list->symbol == "<lista_parametara>")
    {
        process_lista_parametara(list);
        for (SymbolTableEntry *param : list->params)
            params.push_back(param->type);
    }

    node->typeInfo = TypeUtils::makeFunctionType(returnType.getBaseType(), params);

    if (existingFunc && existingFunc->type != node->entry.type || currentScope != SA->globalScope())
        return reportError(node);

    currentScope->insert(node->name, &(node->entry))->isDefined = true;

    currentScope = currentScope->createChildScope();
    for (SymbolTableEntry *param : list->params)
        currentScope->insert(param->name, param);

    SA->naredbaProcessor.process_slozena_naredba(body);
    currentScope = currentScope->parent;
}

// <lista_parametara> ::= <deklaracija_parametra>
// <lista_parametara> ::= <lista_parametara> ZAREZ <deklaracija_parametra>
void SemanticAnalyzer::DeklaracijaProcessor::process_lista_parametara(Node *node)
{
    cerr << "<lista_parametara>" << endl;

    if (node->children.size() == 1)
    {
        Node *deklaracija = node->children[0];
        process_deklaracija_parametra(deklaracija);

        node->params = {&(deklaracija->entry)};
    }
    else if (node->children.size() == 3)
    {
        Node *deklaracija = node->children[2];
        process_deklaracija_parametra(deklaracija);

        Node *lista = node->children[0];
        process_lista_parametara(lista);

        for (const SymbolTableEntry *param : lista->params)
            if (param->name == deklaracija->name)
                return reportError(node);

        node->params = lista->params;
        node->params.push_back(&(deklaracija->entry));
    }
}

// <deklaracija_parametra> ::= <ime_tipa> IDN
// <deklaracija_parametra> ::= <ime_tipa> IDN L_UGL_ZAGRADA D_UGL_ZAGRADA
void SemanticAnalyzer::DeklaracijaProcessor::process_deklaracija_parametra(Node *node)
{
    cerr << "<lista_deklaracija_parametra>" << endl;

    Node *type_name = node->children[0];
    SA->izrazProcessor.process_ime_tipa(type_name);

    TypeInfo paramType = type_name->typeInfo;
    node->name = node->children[1]->lexicalUnit;

    if (paramType.isVoid())
        reportError(node);

    if (node->children.size() == 3)
        paramType = TypeUtils::makeArrayType(paramType.getBaseType(), paramType.isConst());

    node->typeInfo = paramType;
}

// <lista_deklaracija> ::= <deklaracija>
// <lista_deklaracija> ::= <lista_deklaracija> <deklaracija>
void SemanticAnalyzer::DeklaracijaProcessor::process_lista_deklaracija(Node *node)
{
    cerr << "<lista_deklaracija>" << endl;
    processAll(node);
}

// <deklaracija> ::= <ime_tipa> <lista_init_deklaratora> TOCKAZAREZ
void SemanticAnalyzer::DeklaracijaProcessor::process_deklaracija(Node *node)
{
    cerr << "<deklaracija>" << endl;

    Node *baseType = node->children[0];
    Node *init_list = node->children[1];

    SA->izrazProcessor.process_ime_tipa(baseType);

    init_list->typeInfo = baseType->typeInfo;
    process_lista_init_deklaratora(init_list);
}

// <lista_init_deklaratora>1 ::= <init_deklarator>
// <lista_init_deklaratora>1 ::= <lista_init_deklaratora>2 ZAREZ <init_deklarator>
void SemanticAnalyzer::DeklaracijaProcessor::process_lista_init_deklaratora(Node *node)
{
    cerr << "<lista_init_deklaratora>" << endl;

    if (node->children.size() == 1)
    {
        Node *deklarator = node->children[0];

        deklarator->typeInfo = node->typeInfo;
        process_init_deklarator(deklarator);
    }
    else if (node->children.size() == 3)
    {
        Node *lista = node->children[0];
        Node *deklarator = node->children[2];

        lista->typeInfo = node->typeInfo;
        process_lista_init_deklaratora(lista);

        deklarator->typeInfo = node->typeInfo;
        process_init_deklarator(deklarator);
    }
}

// <init_deklarator> ::= <izravni_deklarator>
// <init_deklarator> ::= <izravni_deklarator> OP_PRIDRUZI <inicijalizator>
void SemanticAnalyzer::DeklaracijaProcessor::process_init_deklarator(Node *node)
{
    cerr << "<init_deklarator>" << endl;

    Node *var = node->children[0];

    var->typeInfo = node->typeInfo;
    process_izravni_deklarator(var);

    if (node->size() == 1)
    {
        if (var->typeInfo.isConst())
            reportError(node);
    }
    else if (node->size() == 3)
    {
        Node *initializer = node->children[2];
        SA->izrazProcessor.process_izraz_pridruzivanja(initializer);

        if (TypeUtils::isNumericType(var->typeInfo))
        {
            if (initializer->typeInfo.canImplicitlyConvertTo(var->typeInfo))
                return;
        }
        else if (var->typeInfo.isArray())
        {
            if (initializer->arraySize <= node->arraySize)
            {
                if (Validator::isCharArray(initializer) != -1)
                    return;

                for (SymbolTableEntry *param : initializer->params)
                    if (!(param->type.canImplicitlyConvertTo(var->typeInfo.getBaseType())))
                        return reportError(node);
                return;
            }
        }
        reportError(node);
    }
}

// <izravni_deklarator> ::= IDN
// <izravni_deklarator> ::= IDN L_UGL_ZAGRADA BROJ D_UGL_ZAGRADA
// <izravni_deklarator> ::= IDN L_ZAGRADA (KR_VOID | <lista_parametara>) D_ZAGRADA
void SemanticAnalyzer::DeklaracijaProcessor::process_izravni_deklarator(Node *node)
{
    cerr << "<izravni_deklarator>" << endl;

    node->name = node->children[0]->lexicalUnit;

    cerr << node->typeInfo.toString() << endl;

    // Value and array types
    if ((node->size() == 1 || node->children[1]->symbol == "L_UGL_ZAGRADA"))
    {
        if (node->typeInfo == TypeInfo::VOID || currentScope->localLookup(node->name))
            return reportError(node);

        if (node->size() > 1)
            if (!Validator::validateArraySize(node->children[2]->lexicalUnit, node->arraySize))
                return reportError(node);
            else
                node->typeInfo = TypeUtils::makeArrayType(node->typeInfo.baseType);
    }
    // Functions
    else if (node->children[1]->symbol == "L_ZAGRADA")
    {
        Node *list = node->children[2];
        vector<TypeInfo> params;

        if (list->symbol == "<lista_parametara>")
        {
            process_lista_parametara(list);
            for (SymbolTableEntry *param : list->params)
                params.push_back(param->type);
        }

        node->typeInfo = TypeUtils::makeFunctionType(node->typeInfo.getBaseType(), params);
    }
    cerr << node->typeInfo.toString() << endl;
    currentScope->insert(node->name, &(node->entry));
}

// <inicijalizator> ::= <izraz_pridruzivanja>
// <inicijalizator> ::= L_VIT_ZAGRADA <lista_izraza_pridruzivanja> D_VIT_ZAGRADA
void SemanticAnalyzer::DeklaracijaProcessor::process_inicijalizator(Node *node)
{
    cerr << "<inicijalizator>" << endl;

    if (node->size() == 1)
    {
        Node *expr = node->children[0];
        SA->izrazProcessor.process_izraz_pridruzivanja(expr);

        node->arraySize = Validator::isCharArray(expr);
        if (node->arraySize == -1)
            node->typeInfo = expr->typeInfo;
    }
    else if (node->size() == 3)
    {
        Node *list = node->children[1];

        SA->izrazProcessor.process_izraz_pridruzivanja(list);

        node->arraySize = list->arraySize;
        node->params = list->params;
    }
}
