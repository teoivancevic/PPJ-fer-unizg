#include "Processors.hpp"

class SemanticAnalyzer
{
    friend Processor;

private:
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
        semanticAnalysis(root);

        // 3. Final checks
        checkMainExists();
        checkAllFunctionsDefined();
    }

private:
    void semanticAnalysis(Node *node);

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

#pragma region helper_functions

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

#pragma region special_checks

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

#pragma endregion special_checks

#pragma endregion helper_functions
};

void SemanticAnalyzer::semanticAnalysis(Node *node)
{
    if (!node)
        return;

    // First process this node based on its type
    if (node->symbol == "<prijevodna_jedinica>")
    {
        naredbaProcessor.process_prijevodna_jedinica(node);
    }

#pragma region expression_processing

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

#pragma endregion expression_processing

#pragma region statement_processing

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

#pragma endregion statement_processing

#pragma region declaration_processing

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

#pragma endregion declaration_processing

    for (Node *child : node->children)
        semanticAnalysis(child);
}

int main()
{
    Node *root = TreeUtils::buildTree();

    // Print the tree to verify the structure
    if (root != nullptr)
    {
        // cout << "\nConstructed Tree:\n";
        // printTree(root);

        // Create and run semantic analyzer
        SemanticAnalyzer analyzer(root);
        analyzer.analyze();

        // Clean up
        delete root;
    }

    return 0;
}