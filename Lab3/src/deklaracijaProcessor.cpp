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

void SemanticAnalyzer::DeklaracijaProcessor::process_izravni_deklarator(Node *node)
{
    if (!node || node->children.empty())
        return;

    string name = node->children[0]->content; // IDN node

    // Check for existing declaration in current scope
    if (currentScope->lookup(name))
    {
        reportError(node);
    }

    if (node->children.size() == 1)
    {
        // Simple variable declaration
        node->typeInfo = node->typeInfo; // Inherit from parent
    }
    else if (node->children.size() == 4 && node->children[1]->symbol == "L_UGL_ZAGRADA")
    {
        // Array declaration
        int size = stoi(node->children[2]->content);
        if (size <= 0 || size > 1024)
        {
            reportError(node);
        }
        node->typeInfo = TypeUtils::makeArrayType(node->typeInfo.getBaseType(),
                                                  node->typeInfo.isConst());
    }
    else if (node->children.size() >= 4 && node->children[1]->symbol == "L_ZAGRADA")
    {
        // Function declaration
        if (node->children[2]->symbol == "<lista_parametara>")
        {
            process_lista_parametara(node->children[2]);
            // Convert string param types to TypeInfo
            for (const auto &paramType : node->children[2]->typeInfo.functionParams)
            {
                // Convert string to TypeInfo...
            }
        }
        node->typeInfo = TypeUtils::makeFunctionType(node->typeInfo.getBaseType(), {});
    }

    // Add to symbol table
    SymbolTableEntry entry;
    entry.name = name;
    entry.type = node->typeInfo;
    currentScope->insert(name, entry);
}

void SemanticAnalyzer::DeklaracijaProcessor::process_inicijalizator(Node *node)
{
    if (!node || node->children.empty())
        return;

    if (node->children.size() == 1)
    {
        // Single expression initializer
        IzrazProcessor izrazProcessor(SA);
        izrazProcessor.process_izraz_pridruzivanja(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
    }
    else
    {
        // Array initializer
        IzrazProcessor izrazProcessor(SA);
        for (auto *child : node->children)
        {
            if (child->symbol == "<izraz_pridruzivanja>")
            {
                izrazProcessor.process_izraz_pridruzivanja(child);
                // Collect types for array elements...
            }
        }
        // Set array initializer type info...
    }
}