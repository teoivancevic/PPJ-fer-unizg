#include "SemanticAnalyzer.hpp"

using namespace TreeUtils;
using namespace TypeUtils;

void SemanticAnalyzer::DeklaracijaProcessor::process_definicija_funkcije(Node *node)
{
    // Extract actual identifier name from IDN node
    cerr << "DEBUG: Processing function definition" << endl;
    
    // Process return type
    cerr << "DEBUG: Processing function return type" << endl;
    SA->izrazProcessor.process_ime_tipa(node->children[0]);
    TypeInfo returnType = node->children[0]->typeInfo;
    cerr << "DEBUG: Function return type is: " << returnType.toString() << endl;
    
    // Extract actual identifier name from IDN node
    string funcName = node->children[1]->lexicalUnit;
    cerr << "DEBUG: Function name is: " << funcName << endl;

    // Store return type in the node itself
    node->typeInfo = returnType;


    string actualName = extractIdentifier(funcName);

    // Process return type from <ime_tipa> node
    //process_ime_tipa(node->children[0]);
    // TypeInfo returnType = node->children[0]->typeInfo;

    // Check return type isn't const-qualified
    if (returnType.isConst())
    {
        reportError(node);
    }

    // Look up any previous declarations
    auto *existingFunc = currentScope->lookup(actualName);
    if (existingFunc && existingFunc->isDefined)
    {
        reportError(node); // Function already defined
    }

    // Create function type info
    TypeInfo functionType;
    functionType.baseType = returnType.baseType;
    
    // Handle parameters
    if (node->children[3]->content.find("KR_VOID") == 0)
    {
        // Function with void parameters
        functionType = TypeUtils::makeFunctionType(returnType.baseType, {TypeInfo::VOID});
    }
    else if (node->children[3]->symbol == "<lista_parametara>")
    {
        // Process parameter list
        process_lista_parametara(node->children[3]);
        functionType = TypeUtils::makeFunctionType(
            returnType.baseType,
            node->children[3]->typeInfo.getFunctionParams()
        );
    }

    // Verify against previous declaration if exists
    if (existingFunc)
    {
        if (existingFunc->type != functionType)
        {
            reportError(node);
        }
        // Update existing entry to mark as defined
        existingFunc->isDefined = true;
    }
    else
    {
        // Create new function entry
        SymbolTableEntry funcEntry;
        funcEntry.name = actualName;
        funcEntry.type = functionType;
        funcEntry.isDefined = true;
        
        // Add to symbol table
        if (!currentScope->insert(actualName, funcEntry))
        {
            reportError(node);
        }
    }

    // Store function return type for use in return statement validation
    node->typeInfo = returnType;

    // Create new scope for function body
    auto prevScope = currentScope;
    currentScope = currentScope->createChildScope();

    // If we have parameters, add them to the new scope
    if (node->children[3]->symbol == "<lista_parametara>")
    {
        process_lista_parametara(node->children[3]);
    }

    // Process function body
    if (node->children.back()->symbol == "<slozena_naredba>")
    {
        NaredbaProcessor naredbaProcessor(SA);
        naredbaProcessor.process_slozena_naredba(node->children.back());
    }

    // Restore previous scope
    currentScope = prevScope;
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
    cerr << "DEBUG: Entering process_deklaracija_parametra\n";

    if (!node || node->children.size() < 2) {
        cerr << "DEBUG: Invalid node or insufficient children in process_deklaracija_parametra\n";
        if (!node) cerr << "DEBUG: Node is null\n";
        if (node) cerr << "DEBUG: Node children count: " << node->children.size() << endl;
        return;
    }

    TypeInfo paramType = node->children[0]->typeInfo; // <ime_tipa> node
    string paramName = node->children[1]->content;    // IDN node

    cerr << "DEBUG: Processing parameter with type: " << node->children[0]->symbol 
         << " and name: " << paramName << endl;

    // Check parameter isn't void
    if (paramType.isVoid()) {
        cerr << "DEBUG: Parameter type is void, reporting error\n";
        reportError(node);
        return;
    }

    // Handle array parameter if present
    if (node->children.size() > 2 && node->children[2]->symbol == "L_UGL_ZAGRADA") {
        cerr << "DEBUG: Detected array parameter for name: " << paramName << endl;
        paramType = TypeUtils::makeArrayType(paramType.getBaseType(), paramType.isConst());
        cerr << "DEBUG: Updated parameter type to array: " << paramType.toString() << endl;
    }

    // Store parameter info
    cerr << "DEBUG: Storing parameter info for: " << paramName 
         << ", type: " << paramType.toString() << endl;
    node->typeInfo.functionParams.push_back(paramType);

    // Add parameter to current scope
    SymbolTableEntry paramEntry;
    paramEntry.name = paramName;
    paramEntry.type = paramType;

    cerr << "DEBUG: Checking redeclaration for parameter: " << paramName << endl;
    if (currentScope->lookup(paramName)) {
        cerr << "DEBUG: Redeclaration detected for parameter: " << paramName << endl;
        reportError(node);
        return;
    }

    cerr << "DEBUG: Inserting parameter into scope: " << paramName << endl;
    currentScope->insert(paramName, paramEntry);

    cerr << "DEBUG: Successfully processed parameter: " << paramName << endl;
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
    cerr << "DEBUG: === Starting deklaracija ===" << endl;
    cerr << "DEBUG: Node children size: " << node->children.size() << endl;
    
    // First child should be ime_tipa
    cerr << "DEBUG: First child symbol: " << node->children[0]->symbol << endl;
    
    // Check if const qualified
    bool isConst = false;
    Node* specTypeNode;
    
    if (node->children[0]->children[0]->content.find("KR_CONST") == 0) {
        isConst = true;
        specTypeNode = node->children[0]->children[1];
    } else {
        specTypeNode = node->children[0]->children[0];
    }
    
    // Process specifikator_tipa
    string typeSpecifier = specTypeNode->children[0]->content;
    cerr << "DEBUG: Found type specifier: " << typeSpecifier 
         << " isConst: " << isConst << endl;
    
    // Set base type
    TypeInfo baseType;
    if (typeSpecifier.find("KR_CHAR") == 0) {
        baseType = TypeInfo(BasicType::CHAR, isConst);
    } else if (typeSpecifier.find("KR_INT") == 0) {
        baseType = TypeInfo(BasicType::INT, isConst);
    } else if (typeSpecifier.find("KR_VOID") == 0) {
        baseType = TypeInfo(BasicType::VOID, isConst);
    }
    
    node->children[0]->typeInfo = baseType;
    cerr << "DEBUG: Set type to: " << baseType.toString() << endl;
    
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

    node->children[0]->typeInfo = node->typeInfo;
    process_izravni_deklarator(node->children[0]);

    // Check if there's an initializer
    if (node->children.size() > 1)
    {
        process_inicijalizator(node->children[2]);

        // For array initialization, check size and type compatibility
        if (node->children[0]->typeInfo.isArray()) {
            // Array initialization size check should not error if initializer
            // is smaller than array declaration
            if (node->children[2]->arraySize <= node->children[0]->arraySize) {
                // Types are compatible
                return;
            }
        }
        // For non-array initialization
        else {
            if (node->children[0]->typeInfo.getBaseType() == BasicType::CHAR)
            {
                if (node->children[2]->typeInfo.getBaseType() != BasicType::CHAR)
                {
                    reportError(node);
                    return;
                }
            }
            else if (!node->children[2]->typeInfo.canImplicitlyConvertTo(node->children[0]->typeInfo))
            {
                reportError(node);
                return;
            }
        }
    }
    else
    {
        // Check if const variable without initializer
        if (node->children[0]->typeInfo.isConst())
        {
            reportError(node);
            return;
        }
    }
}


void SemanticAnalyzer::DeklaracijaProcessor::process_izravni_deklarator(Node *node) {
    cerr << "DEBUG: === Starting izravni_deklarator ===" << endl;
    cerr << "DEBUG: Node type info: " << node->typeInfo.toString() << endl;

    if (!node || node->children.empty()) {
        cerr << "DEBUG: Empty node" << endl;
        reportError(node);
        return;
    }

    string name = node->children[0]->content;
    cerr << "DEBUG: Processing identifier: " << name << endl;
        
    
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
    // <izravni_deklarator> ::= IDN L_UGL_ZAGRADA BROJ D_UGL_ZAGRADA
    else if (node->children.size() == 4 && node->children[1]->content.find("L_UGL_ZAGRADA") == 0) {
        cerr << "DEBUG: Processing array declaration" << endl;
        
        // Check if type is void
        if (node->typeInfo.isVoid()) {
            reportError(node);
            return;
        }

        // Parse array size
        int size;
        try {
            size = stoi(node->children[2]->lexicalUnit);
            cerr << "DEBUG: Array size = " << size << endl;
        } catch (...) {
            reportError(node);
            return;
        }

        // Validate array size (positive and <= 1024)
        // This check needs to happen BEFORE any other processing
        if (size <= 0 || size > 1024) {
            reportError(node);  // Report error immediately for invalid size
            return;
        }

        // Only proceed with the rest if size is valid
        if (currentScope->lookup(name)) {
            reportError(node);
            return;
        }

        // Create array type and add to symbol table
        SymbolTableEntry entry;
        entry.name = name;
        entry.type = TypeUtils::makeArrayType(node->typeInfo.getBaseType(), node->typeInfo.isConst());
        entry.arraySize = size;
        node->arraySize = size;
        currentScope->insert(name, entry);
    }
    // <izravni_deklarator> ::= IDN L_ZAGRADA KR_VOID D_ZAGRADA
    else if (node->children.size() == 4 && node->children[1]->content.find("L_ZAGRADA") == 0) {
        string name = extractIdentifier(node->children[0]->lexicalUnit);
        cerr << "DEBUG: Adding function declaration for " << name << endl;
        
        // Create function type with void parameter
        TypeInfo funcType = TypeUtils::makeFunctionType(
            node->typeInfo.getBaseType(),
            {TypeInfo::VOID}
        );
        
        // Find the global scope
        SymbolTable* globalScope = currentScope;
        while (globalScope->parent != nullptr) {
            globalScope = globalScope->parent;
        }
        
        // Add to global symbol table instead of current scope
        SymbolTableEntry entry;
        entry.name = name;
        entry.type = funcType;
        entry.isDefined = false;
        
        bool inserted = globalScope->insert(name, entry);
        cerr << "DEBUG: Function " << name << " isDefined=" << entry.isDefined 
            << " in global scope" << endl;
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
            entry.isDefined = false;  // Explicitly mark as not defined
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
        
        // Check if this is a string literal initialization
        if (node->children[0]->children[0]->children[0]->children[0]->children[0]->children[0]->children[0]->symbol == "NIZ_ZNAKOVA") {
            string strLiteral = node->children[0]->children[0]->children[0]->children[0]->children[0]->children[0]->children[0]->lexicalUnit;
            size_t length = strLiteral.length() - 2 + 1; // -2 for quotes, +1 for null terminator
            
            vector<TypeInfo> types(length, TypeInfo(BasicType::CHAR));
            node->typeInfo = TypeInfo(BasicType::VOID, types);
            node->arraySize = length;
        } else {
            node->typeInfo = node->children[0]->typeInfo;
            node->arraySize = 1;
        }
    }
    // <inicijalizator> ::= L_VIT_ZAGRADA <lista_izraza_pridruzivanja> D_VIT_ZAGRADA
    else if (node->children.size() == 3) {
        process_lista_izraza_pridruzivanja(node->children[1]);
        node->typeInfo = node->children[1]->typeInfo;
        node->arraySize = node->children[1]->arraySize;
    }
}


void SemanticAnalyzer::DeklaracijaProcessor::process_lista_izraza_pridruzivanja(Node *node) {
    if (!node) return;

    if (node->children.size() == 1) {
        // Single expression
        SA->izrazProcessor.process_izraz_pridruzivanja(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->arraySize = 1;
        
        // Create single element vector of types
        vector<TypeInfo> types = {node->typeInfo};
        node->typeInfo = TypeInfo(BasicType::VOID, types);
    }
    else if (node->children.size() == 3) {
        process_lista_izraza_pridruzivanja(node->children[0]);
        SA->izrazProcessor.process_izraz_pridruzivanja(node->children[2]);
        
        // Get existing types from left side
        vector<TypeInfo> types = node->children[0]->typeInfo.getFunctionParams();
        // Add new type
        types.push_back(node->children[2]->typeInfo);
        
        node->typeInfo = TypeInfo(BasicType::VOID, types);
        node->arraySize = node->children[0]->arraySize + 1;
    }
}