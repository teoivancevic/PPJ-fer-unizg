#include "utils.hpp"
#include "izrazProcessor.hpp"
#include "naredbaProcessor.hpp"
#include "deklaracijaProcessor.hpp"


#pragma region Claude_definitions




class SemanticAnalyzer {
private:
    Node* root;
    SymbolTable* currentScope;
    IzrazProcessor izrazProcessor;
    NaredbaProcessor naredbaProcessor;
    DeklaracijaProcessor deklaracijaProcessor;
    
    // Helper functions
    bool isNewScope(Node* node) {
        return node->symbol == "<slozena_naredba>" || 
            node->symbol == "<definicija_funkcije>";
    }

    bool isDeclaration(Node* node) {
        return node->symbol == "<deklaracija>" || 
            node->symbol == "<definicija_funkcije>";
    }

    void recordDeclaration(Node* node) {
        // Record the declaration in current symbol table
        if (node->symbol == "deklaracija") {
            // Handle variable declaration
            SymbolTableEntry entry;
            // Fill in entry details based on node...
            currentScope->insert(node->content, entry);
        }
        else if (node->symbol == "definicija_funkcije") {
            // Handle function definition
            SymbolTableEntry entry;
            entry.isFunction = true;
            // Fill in other entry details...
            currentScope->insert(node->content, entry);
        }
    }

    void checkAssignment(Node* node) {
        // Check assignment expression
        // Implement type checking and other semantic rules
    }

    void checkMainExists() {
        // Check if main function exists with correct signature
        SymbolTableEntry* mainEntry = currentScope->lookup("main");
        if (!mainEntry || !mainEntry->isFunction || mainEntry->type != "int" || 
            mainEntry->paramTypes != vector<string>{"void"}) {
            cout << "main" << endl;
            exit(0);
        }
    }

    void checkAllFunctionsDefined() {
        // Check if all declared functions are defined
        // Implement function definition checking
        // Iterate through all symbols in the global scope
        for (const auto& entry : currentScope->getSymbols()) {  // You'll need to add a getter for symbols
            const string& name = entry.first;
            const SymbolTableEntry& symbol = entry.second;
            
            // Check only function entries
            if (symbol.isFunction && !symbol.isDefined) {
                // Found a function declaration without definition
                cout << name << endl;
                exit(0);
            }
        }
    }

public:
    SemanticAnalyzer(Node* rootNode) : 
        root(rootNode), 
        currentScope(nullptr),
        izrazProcessor(currentScope),
        naredbaProcessor(currentScope),
        deklaracijaProcessor(currentScope)
    {}

    void analyze() {
        // 1. Build symbol tables
        currentScope = new SymbolTable(nullptr); // Global scope
        izrazProcessor = IzrazProcessor(currentScope);
        naredbaProcessor = NaredbaProcessor(currentScope);
        deklaracijaProcessor = DeklaracijaProcessor(currentScope);
        buildSymbolTables(root);


        // 2. Perform semantic analysis
        semanticAnalysis(root);
        
        // 3. Final checks
        // Check for main immediately after building tables
        if (!currentScope->lookup("main")) {
            cout << "main" << endl;
            exit(0);
        }

        // checkMainExists();              // TODO: @teo kj je ovo
        checkAllFunctionsDefined();     // TODO: @teo kj je ovo
        
        
    }
    
    // 1. First Pass - Build Symbol Tables
    void buildSymbolTables(Node* node) {
        if (!node) return;
        
        // Create new scope for functions and blocks
        if (isNewScope(node)) {
            SymbolTable* newScope = new SymbolTable(currentScope);
            currentScope = newScope;
        }
        
        // Record declarations
        if (isDeclaration(node)) {
            recordDeclaration(node);
        }
        
        // Recurse on children
        for (Node* child : node->children) {
            buildSymbolTables(child);
        }
        
        // Restore previous scope
        if (isNewScope(node)) {
            currentScope = currentScope->getParent();
        }
    }
    
    // 2. Second Pass - Semantic Analysis
    void semanticAnalysis(Node* node) {
        if (!node) return;
        
        // Check each type of node according to semantic rules
        // if (node->symbol == "izraz_pridruzivanja") {
        //     checkAssignment(node);
        // }
        // ... other checks
        // cerr << node->symbol << endl;

        
        if (node->symbol == "<prijevodna_jedinica>") {
        // Check global declarations
        }
        else if (node->symbol == "<primarni_izraz>") {
            izrazProcessor.process_primarni_izraz(node);
        }
        else if (node->symbol == "<izraz_pridruzivanja>") {
            izrazProcessor.process_izraz_pridruzivanja(node);
        }
        else if (node->symbol == "<izraz>") {
            izrazProcessor.process_izraz(node);
        }
        else if (node->symbol == "<izraz_pridruzivanja>") {
            checkAssignment(node);
        }

        // 4.4.5 - Naredbena struktura programa
        else if (node->symbol == "<slozena_naredba>") {
            // not implemented
        }
        else if (node->symbol == "<lista_naredbi>") {
            // not implemented
        }
        else if (node->symbol == "<naredba>") {
            // not implemented
        }
        else if (node->symbol == "<izraz_naredba>") {
            // not implemented
        }
        else if (node->symbol == "<naredba_grananja>") {
            // not implemented
        }
        else if (node->symbol == "<naredba_petlje>") {
            // not implemented
        }
        else if (node->symbol == "<naredba_skoka>") {
            // not implemented
            naredbaProcessor.process_naredba_skoka(node);
        }
        else if (node->symbol == "<prijevodna_jedinica>") {
            // not implemented
        }
        else if (node->symbol == "<vanjska_deklaracija>") {
            // not implemented
        }
        
        

        // 4.4.6 - Deklaracije i definicije
        else if (node->symbol == "<definicija_funkcije>") {
            // not implemented
        }
        else if (node->symbol == "<lista_parametara>") {
            // not implemented
        }
        else if (node->symbol == "<deklaracija_parametra>") {
            // not implemented
        }
        else if (node->symbol == "<lista_deklaracija>") {
            // not implemented
        }
        else if (node->symbol == "<deklaracija>") {
            // Validate variable/function declarations
        }
        
        // Recurse on children
        for (Node* child : node->children) {
            semanticAnalysis(child);
        }
    }
};



#pragma endregion Claude_definitions



int main() {
    Node* root = buildTree();
    
    // Print the tree to verify the structure
    if (root != nullptr) {
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