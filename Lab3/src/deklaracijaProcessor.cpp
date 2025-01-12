#include "SemanticAnalyzer.hpp"

using namespace TreeUtils;
using namespace TypeUtils;

void SemanticAnalyzer::DeklaracijaProcessor::process_definicija_funkcije(Node *node)
{
    // Get function name and return type
    string funcName = node->children[1]->content;      // IDN node
    TypeInfo returnType = node->children[0]->typeInfo; // <ime_tipa> node

    // Check return type isn't const-qualified
    if (returnType.isConst())
    {
        reportError(node);
    }

    // Look up any previous declarations
    auto *existingFunc = currentScope->lookup(funcName);
    if (existingFunc && existingFunc->isDefined)
    {
        reportError(node); // Function already defined
    }

    // Create function entry
    SymbolTableEntry funcEntry;
    funcEntry.name = funcName;
    funcEntry.isDefined = true;
    funcEntry.type.returnType = returnType.baseType;

    // Process parameters if present
    if (node->children.size() > 4 && node->children[3]->symbol == "<lista_parametara>")
    {
        process_lista_parametara(node->children[3]);
        funcEntry.type.functionParams = node->children[3]->typeInfo.getFunctionParams();
    }

    // Verify against previous declaration if exists
    if (existingFunc)
    {
        if (existingFunc->type != funcEntry.type ||
            existingFunc->type.functionParams != funcEntry.type.functionParams)
        {
            reportError(node);
        }
    }

    // Add to symbol table
    currentScope->insert(funcName, funcEntry);

    // Process function body
    if (node->children.back()->symbol == "<slozena_naredba>")
    {
        NaredbaProcessor naredbaProcessor(SA);
        naredbaProcessor.process_slozena_naredba(node->children.back());
    }
}

void SemanticAnalyzer::DeklaracijaProcessor::process_lista_parametara(Node *node)
{
    if (!node || node->children.empty())
        return;

    if (node->children.size() == 1)
    {
        // Single parameter case
        process_deklaracija_parametra(node->children[0]);
        node->typeInfo.functionParams = node->children[0]->typeInfo.getFunctionParams();
    }
    else if (node->children.size() == 3)
    {
        // Multiple parameters case
        process_lista_parametara(node->children[0]);
        process_deklaracija_parametra(node->children[2]);

        // Combine parameter types
        node->typeInfo.functionParams = node->children[0]->typeInfo.functionParams;
        node->typeInfo.functionParams.insert(node->typeInfo.functionParams.end(),
                                             node->children[2]->typeInfo.functionParams.begin(),
                                             node->children[2]->typeInfo.functionParams.end());

        // Check for duplicate parameter names
        vector<string> paramNames;
        for (auto &child : node->children)
        {
            if (child->symbol == "<deklaracija_parametra>")
            {
                string paramName = child->children[1]->content; // IDN node
                if (find(paramNames.begin(), paramNames.end(), paramName) != paramNames.end())
                {
                    reportError(node);
                }
                paramNames.push_back(paramName);
            }
        }
    }
}

void SemanticAnalyzer::DeklaracijaProcessor::process_deklaracija_parametra(Node *node)
{
    if (!node || node->children.size() < 2)
        return;

    TypeInfo paramType = node->children[0]->typeInfo; // <ime_tipa> node
    string paramName = node->children[1]->content;    // IDN node

    // Check parameter isn't void
    if (paramType.isVoid())
    {
        reportError(node);
    }

    // Handle array parameter if present
    if (node->children.size() > 2 && node->children[2]->symbol == "L_UGL_ZAGRADA")
    {
        paramType = TypeUtils::makeArrayType(paramType.getBaseType(), paramType.isConst());
    }

    // Store parameter info
    node->typeInfo.functionParams.push_back(paramType);

    // Add parameter to current scope
    SymbolTableEntry paramEntry;
    paramEntry.name = paramName;
    paramEntry.type = paramType;
    currentScope->insert(paramName, paramEntry);
}

void SemanticAnalyzer::DeklaracijaProcessor::process_lista_deklaracija(Node *node)
{
    if (!node || node->children.empty())
        return;

    if (node->children.size() == 1)
    {
        process_deklaracija(node->children[0]);
    }
    else
    {
        process_lista_deklaracija(node->children[0]);
        process_deklaracija(node->children[1]);
    }
}

void SemanticAnalyzer::DeklaracijaProcessor::process_deklaracija(Node *node)
{
    if (!node || node->children.size() < 3)
        return;

    // Get base type from <ime_tipa>
    TypeInfo baseType = node->children[0]->typeInfo;

    // Process initializer list with inherited type info
    node->children[1]->typeInfo = baseType;
    process_lista_init_deklaratora(node->children[1]);
}

void SemanticAnalyzer::DeklaracijaProcessor::process_lista_init_deklaratora(Node *node)
{
    if (!node || node->children.empty())
        return;

    if (node->children.size() == 1)
    {
        node->children[0]->typeInfo = node->typeInfo;
        process_init_deklarator(node->children[0]);
    }
    else
    {
        node->children[0]->typeInfo = node->typeInfo;
        node->children[2]->typeInfo = node->typeInfo;
        process_lista_init_deklaratora(node->children[0]);
        process_init_deklarator(node->children[2]);
    }
}

void SemanticAnalyzer::DeklaracijaProcessor::process_init_deklarator(Node *node)
{
    if (!node || node->children.empty())
        return;

    process_izravni_deklarator(node->children[0]);

    // Check if there's an initializer
    if (node->children.size() > 1)
    {
        IzrazProcessor izrazProcessor(SA);
        izrazProcessor.process_izraz_pridruzivanja(node->children[2]);

        // Specific check for char initialization
        if (node->children[0]->typeInfo.getBaseType() == BasicType::CHAR)
        {
            // For char, the initializer type must be exactly char
            // This means `char c = c + 1` is invalid because c + 1 is int
            if (node->children[2]->typeInfo.getBaseType() != BasicType::CHAR)
            {
                reportError(node);
            }
        }
        else
        {
            // For other types, use standard type compatibility
            if (!node->children[2]->typeInfo.canImplicitlyConvertTo(node->children[0]->typeInfo))
            {
                reportError(node);
            }
        }
    }
    else
    {
        // Check if const variable without initializer
        if (node->children[0]->typeInfo.isConst())
        {
            reportError(node);
        }
    }
}

void SemanticAnalyzer::DeklaracijaProcessor::process_izravni_deklarator(Node *node) {
    if (!node || node->children.empty()) {
        reportError(node);
        return;
    }

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