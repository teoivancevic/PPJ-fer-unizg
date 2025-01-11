#include "Data.hpp"

using namespace TreeUtils;

class SemanticAnalyzer
{
private:
    class Validator
    {
    public:
        static bool validateNumConstant(const string &lexeme, TypeInfo &outType)
        {
            try
            {
                int value = stoi(lexeme);
                if (!Constants::isValidNumConstant(value))
                {
                    return false;
                }
                outType = TypeInfo(BasicType::INT);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        static bool validateCharConstant(const string &unit, TypeInfo &outType)
        {
            string charValue = unit.substr(1, unit.length() - 2); // Remove quotes

            if (charValue.length() == 1)
            {
                if (!Constants::isValidNumConstant(static_cast<int>(charValue[0])))
                    return false;
            }
            else if (charValue.length() == 2 && charValue[0] == '\\')
                if (!Constants::isValidEscapeSequence(charValue[1]))
                    return false;
                else
                    return false;

            outType = TypeInfo(BasicType::CHAR);
            return true;
        }

        static bool validateNizZnakova(const string &lexeme, TypeInfo &outType)
        {
            for (size_t i = 0; i < lexeme.length(); i++)
            {
                if (lexeme[i] == '\\')
                {
                    if (i + 1 >= lexeme.length())
                    {
                        return false;
                    }
                    if (!Constants::isValidEscapeSequence(lexeme[i + 1]))
                    {
                        return false;
                    }
                    i++;
                }
                else if (!Constants::isValidNumConstant(static_cast<int>(lexeme[i])))
                {
                    return false;
                }
            }

            outType = TypeUtils::makeArrayType(BasicType::CHAR, true);
            return true;
        }
    };

    class Processor
    {
    public:
        Processor(SemanticAnalyzer *SA) : SA(SA) {}

        void processAll(Node *node)
        {
            for (Node *child : node->children)
                if (!node->isTerminating())
                    SA->process(child);
        }

    protected:
        SemanticAnalyzer *SA;
        SymbolTable *&currentScope = SA->currentScope;
    };

    // Za obradu izraza (4.4.4)
    class IzrazProcessor : Processor
    {
    public:
        using Processor::Processor;

        void process_primarni_izraz(Node *node);
        void process_postfiks_izraz(Node *node);
        void process_lista_argumenata(Node *node);
        void process_unarni_izraz(Node *node);
        void process_unarni_operator(Node *node);
        void process_cast_izraz(Node *node);
        void process_ime_tipa(Node *node);
        void process_specifikator_tipa(Node *node);
        void process_multiplikativni_izraz(Node *node);
        void process_aditivni_izraz(Node *node);
        void process_odnosni_izraz(Node *node);
        void process_jednakosni_izraz(Node *node);
        void process_bin_i_izraz(Node *node);
        void process_bin_xili_izraz(Node *node);
        void process_bin_ili_izraz(Node *node);
        void process_log_i_izraz(Node *node);
        void process_log_ili_izraz(Node *node);
        void process_izraz_pridruzivanja(Node *node);
        void process_izraz(Node *node);
    };

    // Za obradu programske strukture (4.4.5)
    class NaredbaProcessor : Processor
    {
    public:
        using Processor::Processor;

        void process_slozena_naredba(Node *node);
        void process_lista_naredbi(Node *node);
        void process_naredba(Node *node);
        void process_izraz_naredba(Node *node);
        void process_naredba_grananja(Node *node);
        void process_naredba_petlje(Node *node);
        void process_naredba_skoka(Node *node);
        void process_prijevodna_jedinica(Node *node);
    };

    // Za obradu deklaracija i definicija (4.4.6)
    class DeklaracijaProcessor : Processor
    {
    public:
        using Processor::Processor;

        void process_definicija_funkcije(Node *node);
        void process_lista_parametara(Node *node);
        void process_deklaracija_parametra(Node *node);
        void process_lista_deklaracija(Node *node);
        void process_deklaracija(Node *node);
        void process_lista_init_deklaratora(Node *node);
        void process_init_deklarator(Node *node);
        void process_izravni_deklarator(Node *node);
        void process_inicijalizator(Node *node);
        void process_vanjska_deklaracija(Node *node);
    };

    Node *root;
    SymbolTable *currentScope;

    IzrazProcessor izrazProcessor;
    NaredbaProcessor naredbaProcessor;
    DeklaracijaProcessor deklaracijaProcessor;

public:
    SemanticAnalyzer(Node *rootNode) : root(rootNode),
                                       currentScope(new SymbolTable()),
                                       izrazProcessor(this),
                                       naredbaProcessor(this),
                                       deklaracijaProcessor(this) {}

    
    void analyze()
    {
        // 1. First Pass - Build Symbol Tables
        buildSymbolTables(root);

        // 2. Perform semantic analysis
        process(root);

        // 3. Final checks
        checkMainExists();
        checkAllFunctionsDefined();
    }

    void process(Node *node);

private:
    void buildSymbolTables(Node *node)
    {
        if (!node)
            return;

        // Create new scope for functions and blocks
        if (isNewScope(node))
        {
            SymbolTable *newScope = new SymbolTable(currentScope);
            currentScope = newScope;
        }

        // Record declarations
        if (isDeclaration(node))
        {
            recordDeclaration(node);
        }

        // Recurse on children
        for (Node *child : node->children)
        {
            buildSymbolTables(child);
        }

        // Restore previous scope
        if (isNewScope(node))
        {
            currentScope = currentScope->parent;
        }
    }

    bool isNewScope(Node *node)
    {
        return node->symbol == "<slozena_naredba>" ||
               node->symbol == "<definicija_funkcije>";
    }

    bool isDeclaration(Node *node)
    {
        return node->symbol == "<deklaracija>" ||
               node->symbol == "<definicija_funkcije>";
    }

    void recordDeclaration(Node *node)
    {
        // Record the declaration in current symbol table
        if (node->symbol == "<deklaracija>")
        {
            // Handle variable declaration
            SymbolTableEntry entry;
            // Fill in entry details based on node...
            currentScope->insert(node->content, entry);
        }
        else if (node->symbol == "<definicija_funkcije>")
        {
            // Handle function definition
            SymbolTableEntry entry;
            entry.isFunction = true;
            // Fill in other entry details...
            currentScope->insert(node->content, entry);
        }
    }

    void checkAssignment(Node *node)
    {
        // Check assignment expression
        // Implement type checking and other semantic rules
    }

    void checkMainExists()
    {
        SymbolTableEntry *mainEntry = currentScope->lookup("main");
        if (!mainEntry || !mainEntry->isFunction || mainEntry->type != "int" ||
            mainEntry->paramTypes != vector<string>{"void"})
        {
            cout << "main" << endl;
            exit(0);
        }
    }

    void checkAllFunctionsDefined()
    {
        for (const auto &[name, symbol] : currentScope->symbols)
        {
            if (symbol.isFunction && !symbol.isDefined)
            {
                cout << name << endl;
                exit(0);
            }
        }
    }
};

void SemanticAnalyzer::process(Node *node)
{
    if (node->symbol == "<prijevodna_jedinica>")
    {
        naredbaProcessor.process_prijevodna_jedinica(node);
    }

    else if (node->symbol == "<primarni_izraz>")
    {
        izrazProcessor.process_primarni_izraz(node);
    }
    else if (node->symbol == "<postfiks_izraz>")
    {
        izrazProcessor.process_postfiks_izraz(node);
    }
    else if (node->symbol == "<lista_argumenata>")
    {
        izrazProcessor.process_lista_argumenata(node);
    }
    else if (node->symbol == "<unarni_izraz>")
    {
        izrazProcessor.process_unarni_izraz(node);
    }
    else if (node->symbol == "<unarni_operator>")
    {
        izrazProcessor.process_unarni_operator(node);
    }
    else if (node->symbol == "<cast_izraz>")
    {
        izrazProcessor.process_cast_izraz(node);
    }
    else if (node->symbol == "<ime_tipa>")
    {
        izrazProcessor.process_ime_tipa(node);
    }
    else if (node->symbol == "<specifikator_tipa>")
    {
        izrazProcessor.process_specifikator_tipa(node);
    }
    else if (node->symbol == "<multiplikativni_izraz>")
    {
        izrazProcessor.process_multiplikativni_izraz(node);
    }
    else if (node->symbol == "<aditivni_izraz>")
    {
        izrazProcessor.process_aditivni_izraz(node);
    }
    else if (node->symbol == "<odnosni_izraz>")
    {
        izrazProcessor.process_odnosni_izraz(node);
    }
    else if (node->symbol == "<jednakosni_izraz>")
    {
        izrazProcessor.process_jednakosni_izraz(node);
    }
    else if (node->symbol == "<bin_i_izraz>")
    {
        izrazProcessor.process_bin_i_izraz(node);
    }
    else if (node->symbol == "<bin_xili_izraz>")
    {
        izrazProcessor.process_bin_xili_izraz(node);
    }
    else if (node->symbol == "<bin_ili_izraz>")
    {
        izrazProcessor.process_bin_ili_izraz(node);
    }
    else if (node->symbol == "<log_i_izraz>")
    {
        izrazProcessor.process_log_i_izraz(node);
    }
    else if (node->symbol == "<log_ili_izraz>")
    {
        izrazProcessor.process_log_ili_izraz(node);
    }
    else if (node->symbol == "<izraz_pridruzivanja>")
    {
        izrazProcessor.process_izraz_pridruzivanja(node);
    }
    else if (node->symbol == "<izraz>")
    {
        izrazProcessor.process_izraz(node);
    }

    else if (node->symbol == "<slozena_naredba>")
    {
        naredbaProcessor.process_slozena_naredba(node);
    }
    else if (node->symbol == "<lista_naredbi>")
    {
        naredbaProcessor.process_lista_naredbi(node);
    }
    else if (node->symbol == "<naredba>")
    {
        naredbaProcessor.process_naredba(node);
    }
    else if (node->symbol == "<izraz_naredba>")
    {
        naredbaProcessor.process_izraz_naredba(node);
    }
    else if (node->symbol == "<naredba_grananja>")
    {
        naredbaProcessor.process_naredba_grananja(node);
    }
    else if (node->symbol == "<naredba_petlje>")
    {
        naredbaProcessor.process_naredba_petlje(node);
    }
    else if (node->symbol == "<naredba_skoka>")
    {
        naredbaProcessor.process_naredba_skoka(node);
    }

    else if (node->symbol == "<vanjska_deklaracija>")
    {
        deklaracijaProcessor.process_vanjska_deklaracija(node);
    }
    else if (node->symbol == "<definicija_funkcije>")
    {
        deklaracijaProcessor.process_definicija_funkcije(node);
    }
    else if (node->symbol == "<lista_parametara>")
    {
        deklaracijaProcessor.process_lista_parametara(node);
    }
    else if (node->symbol == "<deklaracija_parametra>")
    {
        deklaracijaProcessor.process_deklaracija_parametra(node);
    }
    else if (node->symbol == "<lista_deklaracija>")
    {
        deklaracijaProcessor.process_lista_deklaracija(node);
    }
    else if (node->symbol == "<deklaracija>")
    {
        deklaracijaProcessor.process_deklaracija(node);
    }
}






// <prijevodna_jedinica> ::= <vanjska_deklaracija>
// <prijevodna_jedinica> ::= <prijevodna_jedinica> <vanjska_deklaracija>
void SemanticAnalyzer::NaredbaProcessor::process_prijevodna_jedinica(Node *node) { processAll(node); }

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









// 

// void SemanticAnalyzer::IzrazProcessor::process_primarni_izraz(Node *node)
// {
//     if (!node || (node->children.size() != 1 && node->children.size() != 3))
//     {
//         reportError(node);
//         return;
//     }

//     // Handle parenthesized expression
//     if (node->children.size() == 3)
//     {
//         if (node->children[0]->content.find("L_ZAGRADA") == 0 &&
//             node->children[2]->content.find("D_ZAGRADA") == 0)
//         {
//             process_izraz(node->children[1]);
//             node->typeInfo = node->children[1]->typeInfo;
//             node->isLValue = node->children[1]->isLValue;
//             return;
//         }
//         reportError(node);
//         return;
//     }

//     Node *child = node->children[0];
//     string childContent = child->content;

//     // Handle identifier
//     if (childContent.find("IDN") == 0)
//     {
//         SymbolTableEntry *entry = currentScope->lookup(child->lexicalUnit);
//         if (!entry)
//         {
//             reportError(node);
//             return;
//         }

//         // Set up type information
//         if (entry->type == "int")
//         {
//             node->typeInfo = TypeInfo(BasicType::INT, entry->isConstant);
//         }
//         else if (entry->type == "char")
//         {
//             node->typeInfo = TypeInfo(BasicType::CHAR, entry->isConstant);
//         }
//         else if (entry->type == "void")
//         {
//             node->typeInfo = TypeInfo(BasicType::VOID, entry->isConstant);
//         }
//         else if (entry->isFunction)
//         {
//             // Handle function type
//             vector<TypeInfo> paramTypes;
//             for (const auto &paramType : entry->paramTypes)
//             {
//                 if (paramType == "int")
//                     paramTypes.push_back(TypeInfo(BasicType::INT));
//                 else if (paramType == "char")
//                     paramTypes.push_back(TypeInfo(BasicType::CHAR));
//                 else if (paramType == "void")
//                     paramTypes.push_back(TypeInfo(BasicType::VOID));
//             }
//             BasicType returnType = BasicType::VOID;
//             if (entry->type == "int")
//                 returnType = BasicType::INT;
//             else if (entry->type == "char")
//                 returnType = BasicType::CHAR;
//             node->typeInfo = TypeUtils::makeFunctionType(returnType, paramTypes);
//         }

//         node->isLValue = !entry->isConstant &&
//                          !entry->isFunction &&
//                          entry->arraySize == -1 &&
//                          (entry->type == "int" || entry->type == "char");
//         return;
//     }

//     // Handle other cases (number, char, string constants)
//     else if (childContent.find("BROJ") == 0)
//     {
//         node->typeInfo = TypeInfo(BasicType::INT);
//         node->isLValue = false;
//     }
//     else if (childContent.find("ZNAK") == 0)
//     {
//         node->typeInfo = TypeInfo(BasicType::CHAR);
//         node->isLValue = false;
//     }
//     else if (childContent.find("NIZ_ZNAKOVA") == 0)
//     {
//         node->typeInfo = TypeUtils::makeArrayType(BasicType::CHAR, true);
//         node->isLValue = false;
//     }
//     else
//     {
//         reportError(node);
//     }
// }

void SemanticAnalyzer::IzrazProcessor::process_primarni_izraz(Node *node)
{
    if (!node || (node->children.size() != 1 && node->children.size() != 3))
    {
        reportError(node);
        return;
    }

    // Handle parenthesized expression
    if (node->children.size() == 3)
    {
        if (node->children[0]->content.find("L_ZAGRADA") == 0 &&
            node->children[2]->content.find("D_ZAGRADA") == 0)
        {
            process_izraz(node->children[1]);
            node->typeInfo = node->children[1]->typeInfo;
            node->isLValue = node->children[1]->isLValue;
            return;
        }
        reportError(node);
        return;
    }

    Node *child = node->children[0];
    string childContent = child->content;

    // Handle identifier - validate existence immediately
    if (childContent.find("IDN") == 0)
    {
        SymbolTableEntry *entry = currentScope->lookup(child->lexicalUnit);
        if (!entry)
        {
            reportError(node);
            return;
        }

        if (entry->isFunction)
        {
            reportError(node);
            return;
        }

        // Set type info based on entry type
        if (entry->type == "int")
        {
            node->typeInfo = TypeInfo(BasicType::INT, entry->isConstant);
        }
        else if (entry->type == "char")
        {
            node->typeInfo = TypeInfo(BasicType::CHAR, entry->isConstant);
        }
        else if (entry->type == "void")
        {
            node->typeInfo = TypeInfo(BasicType::VOID, entry->isConstant);
        }
        else if (entry->isFunction)
        {
            vector<TypeInfo> paramTypes;
            for (const auto &paramType : entry->paramTypes)
            {
                if (paramType == "int")
                    paramTypes.push_back(TypeInfo(BasicType::INT));
                else if (paramType == "char")
                    paramTypes.push_back(TypeInfo(BasicType::CHAR));
                else if (paramType == "void")
                    paramTypes.push_back(TypeInfo(BasicType::VOID));
            }
            BasicType returnType = BasicType::VOID;
            if (entry->type == "int")
                returnType = BasicType::INT;
            else if (entry->type == "char")
                returnType = BasicType::CHAR;
            node->typeInfo = TypeUtils::makeFunctionType(returnType, paramTypes);
        }

        node->isLValue = !entry->isConstant &&
                         !entry->isFunction &&
                         entry->arraySize == -1 &&
                         (entry->type == "int" || entry->type == "char");
        return;
    }

    // Handle constants - validate format
    if (childContent.find("BROJ") == 0)
    {
        if (!Validator::validateNumConstant(child->lexicalUnit, node->typeInfo))
        {
            reportError(node);
            return;
        }
        node->isLValue = false;
        return;
    }
    else if (childContent.find("ZNAK") == 0)
    {
        if (!Validator::validateCharConstant(child->lexicalUnit, node->typeInfo))
        {
            reportError(node);
            return;
        }
        node->isLValue = false;
        return;
    }
    else if (childContent.find("NIZ_ZNAKOVA") == 0)
    {
        if (!Validator::validateNizZnakova(child->lexicalUnit, node->typeInfo))
        {
            reportError(node);
            return;
        }
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_postfiks_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <postfiks_izraz> ::= <primarni_izraz>
    if (node->children.size() == 1)
    {
        process_primarni_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // Other postfix expressions have more children
    if (node->children.size() < 2)
    {
        reportError(node);
        return;
    }

    // Get first child's type and values
    process_postfiks_izraz(node->children[0]);

    // <postfiks_izraz> ::= <postfiks_izraz> L_UGL_ZAGRADA <izraz> D_UGL_ZAGRADA
    if (node->children[1]->content.find("L_UGL_ZAGRADA") == 0)
    {
        // Check array access
        if (!TypeUtils::isArrayType(node->children[0]->typeInfo))
        {
            reportError(node);
            return;
        }

        process_izraz(node->children[2]);
        if (!node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        // Result type is the array's element type
        BasicType baseType = node->children[0]->typeInfo.getBaseType();
        bool isConst = node->children[0]->typeInfo.isConst();
        node->typeInfo = TypeInfo(baseType, isConst);
        node->isLValue = !isConst;
        return;
    }

    // <postfiks_izraz> ::= <postfiks_izraz> L_ZAGRADA D_ZAGRADA
    if (node->children[1]->content.find("L_ZAGRADA") == 0 &&
        node->children[2]->content.find("D_ZAGRADA") == 0)
    {
        // Function call with no arguments
        if (!TypeUtils::isFunctionType(node->children[0]->typeInfo))
        {
            reportError(node);
            return;
        }

        // Check if function accepts void
        if (!node->children[0]->typeInfo.isVoidParam())
        {
            reportError(node);
            return;
        }

        node->typeInfo = TypeInfo(node->children[0]->typeInfo.getReturnType());
        node->isLValue = false;
        return;
    }

    // <postfiks_izraz> ::= <postfiks_izraz> L_ZAGRADA <lista_argumenata> D_ZAGRADA
    if (node->children[1]->content.find("L_ZAGRADA") == 0)
    {
        // Function call with arguments
        if (!TypeUtils::isFunctionType(node->children[0]->typeInfo))
        {
            reportError(node);
            return;
        }

        process_lista_argumenata(node->children[2]);

        // Check parameter compatibility
        const auto &params = node->children[0]->typeInfo.getFunctionParams();
        const auto &args = node->children[2]->typeInfo.getFunctionParams();

        if (params.size() != args.size())
        {
            reportError(node);
            return;
        }

        for (size_t i = 0; i < params.size(); i++)
        {
            if (!args[i].canImplicitlyConvertTo(params[i]))
            {
                reportError(node);
                return;
            }
        }

        node->typeInfo = TypeInfo(node->children[0]->typeInfo.getReturnType());
        node->isLValue = false;
        return;
    }

    // <postfiks_izraz> ::= <postfiks_izraz> OP_INC/OP_DEC
    if (node->children[1]->content.find("OP_INC") == 0 ||
        node->children[1]->content.find("OP_DEC") == 0)
    {
        // Check if operand is l-value and numeric type
        if (!node->children[0]->isLValue ||
            !node->children[0]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_lista_argumenata(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <lista_argumenata> ::= <izraz_pridruzivanja>
    if (node->children.size() == 1)
    {
        process_izraz_pridruzivanja(node->children[0]);
        // Create a vector with single argument type
        vector<TypeInfo> params = {node->children[0]->typeInfo};
        node->typeInfo = TypeInfo(BasicType::VOID, params);
        return;
    }

    // <lista_argumenata> ::= <lista_argumenata> ZAREZ <izraz_pridruzivanja>
    if (node->children.size() == 3)
    {
        process_lista_argumenata(node->children[0]);
        process_izraz_pridruzivanja(node->children[2]);

        // Get existing params and add new one
        vector<TypeInfo> params = node->children[0]->typeInfo.getFunctionParams();
        params.push_back(node->children[2]->typeInfo);
        node->typeInfo = TypeInfo(BasicType::VOID, params);
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_unarni_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <unarni_izraz> ::= <postfiks_izraz>
    if (node->children.size() == 1)
    {
        process_postfiks_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <unarni_izraz> ::= (OP_INC | OP_DEC) <unarni_izraz>
    if (node->children[0]->content.find("OP_INC") == 0 ||
        node->children[0]->content.find("OP_DEC") == 0)
    {
        process_unarni_izraz(node->children[1]);

        // Check if operand is l-value and numeric type
        if (!node->children[1]->isLValue ||
            !node->children[1]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    // <unarni_izraz> ::= <unarni_operator> <cast_izraz>
    if (node->children.size() == 2)
    {
        process_cast_izraz(node->children[1]);

        // Check if operand can be converted to int
        if (!node->children[1]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_unarni_operator(Node *node)
{
    // Note: According to lab instructions, no semantic rules need to be checked for unary operators
    // They just generate PLUS, MINUS, OP_TILDA and OP_NEG operators
    // The semantic checking is done in process_unarni_izraz where we ensure operands are of correct type

    // However we should still validate the node structure
    if (!node || node->children.size() != 1)
    {
        reportError(node);
        return;
    }

    string op = node->children[0]->content;
    if (op.find("PLUS") != 0 &&
        op.find("MINUS") != 0 &&
        op.find("OP_TILDA") != 0 &&
        op.find("OP_NEG") != 0)
    {
        reportError(node);
    }
}

void SemanticAnalyzer::IzrazProcessor::process_cast_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <cast_izraz> ::= <unarni_izraz>
    if (node->children.size() == 1)
    {
        process_unarni_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <cast_izraz> ::= L_ZAGRADA <ime_tipa> D_ZAGRADA <cast_izraz>
    if (node->children.size() == 4)
    {
        // Process both sides
        process_ime_tipa(node->children[1]);
        process_cast_izraz(node->children[3]);

        // Get target type from ime_tipa
        TypeInfo targetType = node->children[1]->typeInfo;
        TypeInfo sourceType = node->children[3]->typeInfo;

        // Rules:
        // 1. Cannot cast from function type
        if (TypeUtils::isFunctionType(sourceType))
        {
            // Only report error if the final cast introduces a problem
            if (node->children[3]->symbol == "<unarni_izraz>")
            {
                reportError(node);
                return;
            }
        }

        // 2. Target cannot be void
        if (targetType.isVoid())
        {
            reportError(node);
            return;
        }

        // 3. Cast is only allowed for numeric types
        if (!TypeUtils::isNumericType(sourceType) &&
            !TypeUtils::isNumericType(targetType))
        {
            reportError(node);
            return;
        }

        // Set type and l-value status
        node->typeInfo = targetType;
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_ime_tipa(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <ime_tipa> ::= <specifikator_tipa>
    if (node->children.size() == 1)
    {
        process_specifikator_tipa(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        return;
    }

    // <ime_tipa> ::= KR_CONST <specifikator_tipa>
    if (node->children.size() == 2)
    {
        if (node->children[0]->content.find("KR_CONST") != 0)
        {
            reportError(node);
            return;
        }

        process_specifikator_tipa(node->children[1]);

        // Cannot have const void
        if (node->children[1]->typeInfo.isVoid())
        {
            reportError(node);
            return;
        }

        // Create const-qualified type
        node->typeInfo = TypeInfo(node->children[1]->typeInfo.getBaseType(), true);
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_specifikator_tipa(Node *node)
{
    if (!node || node->children.size() != 1)
    {
        reportError(node);
        return;
    }

    string typeSpecifier = node->children[0]->content;

    // <specifikator_tipa> ::= KR_VOID
    if (typeSpecifier.find("KR_VOID") == 0)
    {
        node->typeInfo = TypeInfo(BasicType::VOID);
        return;
    }
    // <specifikator_tipa> ::= KR_CHAR
    else if (typeSpecifier.find("KR_CHAR") == 0)
    {
        node->typeInfo = TypeInfo(BasicType::CHAR);
        return;
    }
    // <specifikator_tipa> ::= KR_INT
    else if (typeSpecifier.find("KR_INT") == 0)
    {
        node->typeInfo = TypeInfo(BasicType::INT);
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_multiplikativni_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <multiplikativni_izraz> ::= <cast_izraz>
    if (node->children.size() == 1)
    {
        process_cast_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <multiplikativni_izraz> ::= <multiplikativni_izraz> (OP_PUTA | OP_DIJELI | OP_MOD) <cast_izraz>
    if (node->children.size() == 3)
    {
        // Process the operands
        process_multiplikativni_izraz(node->children[0]);
        process_cast_izraz(node->children[2]);

        // Both operands must be implicitly convertible to int
        if (!node->children[0]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)) ||
            !node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        // Check if the operator is valid
        string op = node->children[1]->content;
        if (op.find("OP_PUTA") != 0 &&
            op.find("OP_DIJELI") != 0 &&
            op.find("OP_MOD") != 0)
        {
            reportError(node);
            return;
        }

        // Result is always int and never an l-value
        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_aditivni_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <aditivni_izraz> ::= <multiplikativni_izraz>
    if (node->children.size() == 1)
    {
        process_multiplikativni_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <aditivni_izraz> ::= <aditivni_izraz> (PLUS | MINUS) <multiplikativni_izraz>
    if (node->children.size() == 3)
    {
        // Process the operands
        process_aditivni_izraz(node->children[0]);
        process_multiplikativni_izraz(node->children[2]);

        // Both operands must be implicitly convertible to int
        if (!node->children[0]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)) ||
            !node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        // Check if the operator is valid
        string op = node->children[1]->content;
        if (op.find("PLUS") != 0 && op.find("MINUS") != 0)
        {
            reportError(node);
            return;
        }

        // Result is always int and never an l-value
        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_odnosni_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <odnosni_izraz> ::= <aditivni_izraz>
    if (node->children.size() == 1)
    {
        process_aditivni_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <odnosni_izraz> ::= <odnosni_izraz> (OP_LT | OP_GT | OP_LTE | OP_GTE) <aditivni_izraz>
    if (node->children.size() == 3)
    {
        // Process both operands
        process_odnosni_izraz(node->children[0]);
        process_aditivni_izraz(node->children[2]);

        // Both operands must be implicitly convertible to int
        if (!node->children[0]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)) ||
            !node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        // Validate operator
        string op = node->children[1]->content;
        if (op.find("OP_LT") != 0 &&
            op.find("OP_GT") != 0 &&
            op.find("OP_LTE") != 0 &&
            op.find("OP_GTE") != 0)
        {
            reportError(node);
            return;
        }

        // Result is int (representing boolean) and never an l-value
        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_jednakosni_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <jednakosni_izraz> ::= <odnosni_izraz>
    if (node->children.size() == 1)
    {
        process_odnosni_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <jednakosni_izraz> ::= <jednakosni_izraz> (OP_EQ | OP_NEQ) <odnosni_izraz>
    if (node->children.size() == 3)
    {
        // Process both operands
        process_jednakosni_izraz(node->children[0]);
        process_odnosni_izraz(node->children[2]);

        // Both operands must be implicitly convertible to int
        if (!node->children[0]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)) ||
            !node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        // Validate operator
        string op = node->children[1]->content;
        if (op.find("OP_EQ") != 0 && op.find("OP_NEQ") != 0)
        {
            reportError(node);
            return;
        }

        // Result is int (representing boolean) and never an l-value
        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_bin_i_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <bin_i_izraz> ::= <jednakosni_izraz>
    if (node->children.size() == 1)
    {
        process_jednakosni_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <bin_i_izraz> ::= <bin_i_izraz> OP_BIN_I <jednakosni_izraz>
    if (node->children.size() == 3)
    {
        process_bin_i_izraz(node->children[0]);
        process_jednakosni_izraz(node->children[2]);

        // Both operands must be implicitly convertible to int
        if (!node->children[0]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)) ||
            !node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        // Result is int and never an l-value
        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_bin_xili_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <bin_xili_izraz> ::= <bin_i_izraz>
    if (node->children.size() == 1)
    {
        process_bin_i_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <bin_xili_izraz> ::= <bin_xili_izraz> OP_BIN_XILI <bin_i_izraz>
    if (node->children.size() == 3)
    {
        process_bin_xili_izraz(node->children[0]);
        process_bin_i_izraz(node->children[2]);

        // Both operands must be implicitly convertible to int
        if (!node->children[0]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)) ||
            !node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        // Result is int and never an l-value
        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_bin_ili_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <bin_ili_izraz> ::= <bin_xili_izraz>
    if (node->children.size() == 1)
    {
        process_bin_xili_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <bin_ili_izraz> ::= <bin_ili_izraz> OP_BIN_ILI <bin_xili_izraz>
    if (node->children.size() == 3)
    {
        process_bin_ili_izraz(node->children[0]);
        process_bin_xili_izraz(node->children[2]);

        // Both operands must be implicitly convertible to int
        if (!node->children[0]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)) ||
            !node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        // Result is int and never an l-value
        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_log_i_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <log_i_izraz> ::= <bin_ili_izraz>
    if (node->children.size() == 1)
    {
        process_bin_ili_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <log_i_izraz> ::= <log_i_izraz> OP_I <bin_ili_izraz>
    if (node->children.size() == 3)
    {
        process_log_i_izraz(node->children[0]);
        process_bin_ili_izraz(node->children[2]);

        // Both operands must be implicitly convertible to int
        if (!node->children[0]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)) ||
            !node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        // Result is int and never an l-value
        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_log_ili_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <log_ili_izraz> ::= <log_i_izraz>
    if (node->children.size() == 1)
    {
        process_log_i_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <log_ili_izraz> ::= <log_ili_izraz> OP_ILI <log_i_izraz>
    if (node->children.size() == 3)
    {
        process_log_ili_izraz(node->children[0]);
        process_log_i_izraz(node->children[2]);

        // Both operands must be implicitly convertible to int
        if (!node->children[0]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)) ||
            !node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        // Result is int and never an l-value
        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_izraz_pridruzivanja(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <izraz_pridruzivanja> ::= <log_ili_izraz>
    if (node->children.size() == 1)
    {
        process_log_ili_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <izraz_pridruzivanja> ::= <postfiks_izraz> OP_PRIDRUZI <izraz_pridruzivanja>
    if (node->children.size() == 3)
    {
        // Process both sides of the assignment
        process_postfiks_izraz(node->children[0]);
        process_izraz_pridruzivanja(node->children[2]);

        // Check if left side is an l-value
        if (!node->children[0]->isLValue)
        {
            reportError(node);
            return;
        }

        // Validate operator
        if (node->children[1]->content.find("OP_PRIDRUZI") != 0)
        {
            reportError(node);
            return;
        }

        // Check type compatibility for assignment
        // Right side must be implicitly convertible to left side's type
        if (!node->children[2]->typeInfo.canImplicitlyConvertTo(node->children[0]->typeInfo))
        {
            reportError(node);
            return;
        }

        // The type of the assignment expression is the type of the left operand
        // but it's not an l-value
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // According to 4.4.4, <izraz> can be either:
    // <izraz> ::= <izraz_pridruzivanja>
    // <izraz> ::= <izraz> ZAREZ <izraz_pridruzivanja>

    if (node->children.size() == 1)
    {
        // <izraz> ::= <izraz_pridruzivanja>
        process_izraz_pridruzivanja(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
    }
    else if (node->children.size() == 3)
    {
        // <izraz> ::= <izraz> ZAREZ <izraz_pridruzivanja>
        process_izraz(node->children[0]);
        process_izraz_pridruzivanja(node->children[2]);
        node->typeInfo = node->children[2]->typeInfo; // Type of the last expression
        node->isLValue = false;
    }
    else
    {
        reportError(node);
    }
}














// #include "semantickiAnalizator.cpp"

// using namespace TreeUtils;

void SemanticAnalyzer::DeklaracijaProcessor::process_definicija_funkcije(Node *node)
{
    // Check base case
    if (!node || node->children.size() < 2)
        return;

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
    funcEntry.isFunction = true;
    funcEntry.isDefined = true;
    funcEntry.type = returnType.toString();

    // Process parameters if present
    if (node->children.size() > 4 && node->children[3]->symbol == "<lista_parametara>")
    {
        process_lista_parametara(node->children[3]);
        funcEntry.paramTypes = node->children[3]->paramTypes;
    }

    // Verify against previous declaration if exists
    if (existingFunc)
    {
        if (existingFunc->type != funcEntry.type ||
            existingFunc->paramTypes != funcEntry.paramTypes)
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
        node->paramTypes = node->children[0]->paramTypes;
    }
    else if (node->children.size() == 3)
    {
        // Multiple parameters case
        process_lista_parametara(node->children[0]);
        process_deklaracija_parametra(node->children[2]);

        // Combine parameter types
        node->paramTypes = node->children[0]->paramTypes;
        node->paramTypes.insert(node->paramTypes.end(),
                                node->children[2]->paramTypes.begin(),
                                node->children[2]->paramTypes.end());

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
    node->paramTypes.push_back(paramType.toString());

    // Add parameter to current scope
    SymbolTableEntry paramEntry;
    paramEntry.name = paramName;
    paramEntry.type = paramType.toString();
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
        vector<TypeInfo> paramTypes;
        if (node->children[2]->symbol == "<lista_parametara>")
        {
            process_lista_parametara(node->children[2]);
            // Convert string param types to TypeInfo
            for (const auto &paramType : node->children[2]->paramTypes)
            {
                // Convert string to TypeInfo...
            }
        }
        node->typeInfo = TypeUtils::makeFunctionType(node->typeInfo.getBaseType(), paramTypes);
    }

    // Add to symbol table
    SymbolTableEntry entry;
    entry.name = name;
    entry.type = node->typeInfo.toString();
    entry.isConstant = node->typeInfo.isConst();
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

void SemanticAnalyzer::DeklaracijaProcessor::process_vanjska_deklaracija(Node* node) {
    if (!node) {
        reportError(node);
        return;
    }

    // <vanjska_deklaracija> ::= <deklaracija>
    // <vanjska_deklaracija> ::= <definicija_funkcije>
    if (node->children.size() != 1) {
        reportError(node);
        return;
    }

    if (node->children[0]->symbol == "<deklaracija>") {
        process_deklaracija(node->children[0]);
    } else if (node->children[0]->symbol == "<definicija_funkcije>") {
        process_definicija_funkcije(node->children[0]);
    } else {
        reportError(node);
    }
}
