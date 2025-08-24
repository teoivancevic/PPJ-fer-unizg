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

    string name = node->children[0]->content; // IDN node
    
    // <izravni_deklarator> ::= IDN
    if (node->children.size() == 1) {
        // Check if type is void
        if (node->typeInfo.isVoid()) {
            reportError(node);
            return;
        }
        
        // Check for redeclaration in current scope
        if (currentScope->lookup(name)) {
            reportError(node);
            return;
        }
        
        // Add to symbol table
        SymbolTableEntry entry;
        entry.name = name;
        entry.type = node->typeInfo;
        currentScope->insert(name, entry);
    }
    // <izravni_deklarator> ::= IDN L_UGL_ZAGRADA BROJ D_UGL_ZAGRADA
    else if (node->children.size() == 4 && node->children[1]->content.find("L_UGL_ZAGRADA") == 0) {
        // Check if type is void
        if (node->typeInfo.isVoid()) {
            reportError(node);
            return;
        }

        // Parse array size
        int size;
        try {
            size = stoi(node->children[2]->lexicalUnit);
        } catch (...) {
            reportError(node);
            return;
        }

        // Validate array size (positive and <= 1024)
        if (size <= 0 || size > 1024) {
            reportError(node);
            return;
        }

        // Check for redeclaration
        if (currentScope->lookup(name)) {
            reportError(node);
            return;
        }

        // Create array type and add to symbol table
        SymbolTableEntry entry;
        entry.name = name;
        entry.type = TypeUtils::makeArrayType(node->typeInfo.getBaseType(), node->typeInfo.isConst());
        entry.arraySize = size;
        currentScope->insert(name, entry);
    }
    // <izravni_deklarator> ::= IDN L_ZAGRADA KR_VOID D_ZAGRADA
    else if (node->children.size() == 4 && node->children[1]->content.find("L_ZAGRADA") == 0) {
        // Check for previous declaration in local scope
        auto* existing = currentScope->lookup(name);
        if (existing) {
            // Verify type matches if already declared
            TypeInfo funcType = TypeUtils::makeFunctionType(node->typeInfo.getBaseType(), {TypeInfo::VOID});
            if (existing->type != funcType) {
                reportError(node);
                return;
            }
        } else {
            // Add new function declaration
            SymbolTableEntry entry;
            entry.name = name;
            entry.type = TypeUtils::makeFunctionType(node->typeInfo.getBaseType(), {TypeInfo::VOID});
            currentScope->insert(name, entry);
        }
    }
    // <izravni_deklarator> ::= IDN L_ZAGRADA <lista_parametara> D_ZAGRADA
    else if (node->children.size() == 4) {
        process_lista_parametara(node->children[2]);
        
        auto* existing = currentScope->lookup(name);
        if (existing) {
            // Verify type matches if already declared
            TypeInfo funcType = TypeUtils::makeFunctionType(
                node->typeInfo.getBaseType(),
                node->children[2]->typeInfo.getFunctionParams()
            );
            if (existing->type != funcType) {
                reportError(node);
                return;
            }
        } else {
            // Add new function declaration
            SymbolTableEntry entry;
            entry.name = name;
            entry.type = TypeUtils::makeFunctionType(
                node->typeInfo.getBaseType(),
                node->children[2]->typeInfo.getFunctionParams()
            );
            currentScope->insert(name, entry);
        }
    } else {
        reportError(node);
    }
}

void SemanticAnalyzer::DeklaracijaProcessor::process_inicijalizator(Node *node) {
    if (!node || node->children.empty()) {
        reportError(node);
        return;
    }

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
    // <inicijalizator> ::= <izraz_pridruzivanja>
    if (node->children.size() == 1) {
        IzrazProcessor izrazProcessor(SA);
        izrazProcessor.process_izraz_pridruzivanja(node->children[0]);
        
        // Check if this is a string literal initializing an array
        if (node->children[0]->typeInfo.isArray() && 
            node->children[0]->typeInfo.getBaseType() == BasicType::CHAR) {
            // Get string length (including null terminator)
            size_t length = node->children[0]->lexicalUnit.length() - 2 + 1; // -2 for quotes, +1 for null
            
            // Set array properties
            node->typeInfo = TypeUtils::makeArrayType(BasicType::CHAR, true);
            node->arraySize = length;
            
            vector<TypeInfo> elementTypes(length, TypeInfo(BasicType::CHAR));
            node->typeInfo.functionParams = elementTypes; // Using functionParams to store element types
        } else {
            // Normal initialization
            node->typeInfo = node->children[0]->typeInfo;
        }
    }
    // <inicijalizator> ::= L_VIT_ZAGRADA <lista_izraza_pridruzivanja> D_VIT_ZAGRADA
    else if (node->children.size() == 3) {
        // Process all initializer expressions
        vector<TypeInfo> elementTypes;
        int elementCount = 0;
        
        // Count and validate all initializer expressions
        for (Node* child : node->children[1]->children) {
            if (child->symbol == "<izraz_pridruzivanja>") {
                IzrazProcessor izrazProcessor(SA);
                izrazProcessor.process_izraz_pridruzivanja(child);
                elementTypes.push_back(child->typeInfo);
                elementCount++;
            }
        }
        
        // Store array information
        node->arraySize = elementCount;
        node->typeInfo.functionParams = elementTypes; // Using functionParams to store element types
    } else {
        reportError(node);
    }
}
