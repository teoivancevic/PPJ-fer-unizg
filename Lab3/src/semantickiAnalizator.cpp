#include "Processors.hpp"

class SemanticAnalyzer
{
private:
    Node *root;
    SymbolTable *currentScope;
    IzrazProcessor izrazProcessor;

    // Helper functions
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
        if (node->symbol == "deklaracija")
        {
            // Handle variable declaration
            SymbolTable::Entry entry;
            // Fill in entry details based on node...
            currentScope->insert(node->content, entry);
        }
        else if (node->symbol == "definicija_funkcije")
        {
            // Handle function definition
            SymbolTable::Entry entry;
            entry.type = TypeInfo();
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
        // Check if main function exists with correct signature
        SymbolTable::Entry *mainEntry = currentScope->lookup("main");
        if (!mainEntry || !mainEntry->type.isFunc() || mainEntry->type.getBaseType() != BasicType::INT ||
            mainEntry->type.getFunctionParams() == vector<TypeInfo>{BasicType::VOID})
        {
            cout << "main" << endl;
            exit(0);
        }
    }

    void checkAllFunctionsDefined()
    {
        // Check if all declared functions are defined
        // Implement function definition checking
    }

public:
    SemanticAnalyzer(Node *rootNode) : root(rootNode), currentScope(nullptr), izrazProcessor(currentScope) {}

    void analyze()
    {
        // 1. Build symbol tables
        currentScope = new SymbolTable(nullptr); // Global scope
        izrazProcessor = IzrazProcessor(currentScope);
        buildSymbolTables(root);

        // 2. Perform semantic analysis
        semanticAnalysis(root);

        // 3. Final checks
        // checkMainExists();              // TODO: @teo kj je ovo
        // checkAllFunctionsDefined();     // TODO: @teo kj je ovo
    }

    // 1. First Pass - Build Symbol Tables
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

    // 2. Second Pass - Semantic Analysis
    void semanticAnalysis(Node *node)
    {
        if (!node)
            return;

        // Check each type of node according to semantic rules
        // if (node->symbol == "izraz_pridruzivanja") {
        //     checkAssignment(node);
        // }
        // ... other checks
        // cerr << node->symbol << endl;

        if (node->symbol == "<prijevodna_jedinica>")
        {
            // Check global declarations
        }
        else if (node->symbol == "<primarni_izraz>")
        {
            izrazProcessor.process_primarni_izraz(node);
        }
        else if (node->symbol == "<izraz_pridruzivanja>")
        {
            izrazProcessor.process_izraz_pridruzivanja(node);
        }
        else if (node->symbol == "<izraz>")
        {
            izrazProcessor.process_izraz(node);
        }
        else if (node->symbol == "<izraz_pridruzivanja>")
        {
            checkAssignment(node);
        }
        else if (node->symbol == "<definicija_funkcije>")
        {
            // Validate function definition
        }
        else if (node->symbol == "<deklaracija>")
        {
            // Validate variable/function declarations
        }
        // ... handle other node types

        // Recurse on children
        for (Node *child : node->children)
        {
            semanticAnalysis(child);
        }
    }
};

int main()
{
    Node *root = buildTree();

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