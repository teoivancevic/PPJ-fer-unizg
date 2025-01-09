#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>

using namespace std;


#pragma region Types

// First, define the basic types
enum class BasicType {
    INT,
    CHAR,
    VOID,
};

class TypeInfo;

namespace TypeUtils {
    bool isNumericType(const TypeInfo& type);
    bool isFunctionType(const TypeInfo& type);
    bool isArrayType(const TypeInfo& type);
    bool areTypesCompatible(const TypeInfo& source, const TypeInfo& target);
    string typeToString(const BasicType& type);
}

// A class to represent the complete type system
class TypeInfo {
private:
    BasicType baseType;
    bool isConst_;
    bool isArray_;
    vector<TypeInfo> functionParams;  // For function types
    BasicType returnType;            // For function types

    // Friend declaration for TypeUtils functions individually
    friend bool TypeUtils::isNumericType(const TypeInfo&);
    friend bool TypeUtils::isFunctionType(const TypeInfo&);
    friend bool TypeUtils::isArrayType(const TypeInfo&);
    friend bool TypeUtils::areTypesCompatible(const TypeInfo&, const TypeInfo&);

public:
    // Constructor for simple types
    TypeInfo(BasicType type, bool constQualified = false)
        : baseType(type), isConst_(constQualified), isArray_(false), returnType(type) {}
    
    // Constructor for array types
    TypeInfo(BasicType type, bool constQualified, bool array)
        : baseType(type), isConst_(constQualified), isArray_(array), returnType(type) {}

    // Constructor for function types
    TypeInfo(BasicType returnType, vector<TypeInfo> params)
        : returnType(returnType), functionParams(params), isArray_(false), 
          baseType(BasicType::VOID), isConst_(false) {}

    // Static factory methods
    static TypeInfo makeArrayType(BasicType type, bool constQualified) {
        return TypeInfo(type, constQualified, true);
    }

    static TypeInfo makeFunctionType(BasicType returnType, vector<TypeInfo> params) {
        return TypeInfo(returnType, params);
    }

    // Method to check implicit type conversion (⇠ relation)
    bool canImplicitlyConvertTo(const TypeInfo& target) const {
        return TypeUtils::areTypesCompatible(*this, target);
    }

    // Getters
    BasicType getBaseType() const { return baseType; }
    BasicType getReturnType() const { return returnType; }
    bool isConst() const { return isConst_; }
    bool isArray() const { return isArray_; }
    const vector<TypeInfo>& getFunctionParams() const { return functionParams; }
    bool isVoidParam() const { return functionParams.empty(); }
    bool isVoid() const { return baseType == BasicType::VOID; }

    // String conversion for debugging
    string toString() const {
        string result = TypeUtils::typeToString(baseType);
        if (isConst()) result = "const(" + result + ")";
        if (isArray()) result = "niz(" + result + ")";
        if (TypeUtils::isFunctionType(*this)) {
            result = "funkcija(" + TypeUtils::typeToString(returnType) + ")";
        }
        return result;
    }

    // Add equality operator
    bool operator==(const TypeInfo& other) const {
        return baseType == other.baseType && 
               isConst() == other.isConst() && 
               isArray() == other.isArray() &&
               functionParams == other.functionParams &&
               returnType == other.returnType;
    }

    bool operator!=(const TypeInfo& other) const {
        return !(*this == other);
    }
};

#pragma region Utils
// Forward declare TypeInfo

// Tree node structure
struct Node {
    string symbol;         // Node symbol (A, B, etc.)
    vector<Node*> children;
    string content;        // Content within the node (a1xxx, b2yy, etc.)

    // New semantic-related members
    string type;          // For tracking types (int, char, void, etc.)
    TypeInfo typeInfo;
    bool isLValue;            // For l-value checking
    
    int lineNumber;           // For error reporting
    string lexicalUnit;  // For error reporting
    
    // For array-specific information
    int arraySize;            // For array declarations
    vector<string> paramTypes;  // For function parameters

    Node(string s) : symbol(s), typeInfo(BasicType::VOID) {}  // Initialize typeInfo
    
    ~Node() {
        for (Node* child : children) {
            delete child;
        }
    }
};

namespace TypeUtils {
    string typeToString(const BasicType& type) {
        switch(type) {
            case BasicType::INT: return "int";
            case BasicType::CHAR: return "char";
            case BasicType::VOID: return "void";
            default: return "unknown";
        }
    }

    bool isNumericType(const TypeInfo& type) {
        return type.getBaseType() == BasicType::INT || 
               type.getBaseType() == BasicType::CHAR;
    }

    bool isFunctionType(const TypeInfo& type) {
        return type.getFunctionParams().size() > 0 || 
               (type.getFunctionParams().size() == 0 && type.isVoidParam());
    }

    bool isArrayType(const TypeInfo& type) {
        return type.isArray();
    }

    bool areTypesCompatible(const TypeInfo& source, const TypeInfo& target) {
        
        
        // Same type is always compatible
        if (source == target) return true;

        // Handle void type
        if (source.isVoid() || target.isVoid()) {
            return false;  // void only compatible with itself
        }

        // Handle const relationships (const(T) ⇠ T and T ⇠ const(T))
        if (source.getBaseType() == target.getBaseType()) {
            if (!source.isConst() || target.isConst()) {
                return true;
            }
        }

        // Handle char ⇠ int conversion
        if (source.getBaseType() == BasicType::CHAR && 
            target.getBaseType() == BasicType::INT) {
            return true;
        }

        // Handle array type conversions
        if (isArrayType(source) && isArrayType(target)) {
            // niz(T) ⇠ niz(const(T))
            if (!source.isConst() && target.isConst() && 
                source.getBaseType() == target.getBaseType()) {
                return true;
            }
        }

        // Handle function type compatibility
        if (isFunctionType(source) && isFunctionType(target)) {
            // Check return type
            if (!areTypesCompatible(
                TypeInfo(source.getReturnType()), 
                TypeInfo(target.getReturnType()))) {
                return false;
            }

            // Check parameters
            const auto& sourceParams = source.getFunctionParams();
            const auto& targetParams = target.getFunctionParams();
            
            if (sourceParams.size() != targetParams.size()) {
                return false;
            }

            for (size_t i = 0; i < sourceParams.size(); i++) {
                if (!areTypesCompatible(sourceParams[i], targetParams[i])) {
                    return false;
                }
            }
            return true;
        }

        // Everything else is incompatible
        return false;
   
    }
}
struct SymbolTableEntry {
    string name;
    string type;         // Basic type or function signature, // "int", "char", "void", or function type
    bool isConstant;          // For const-qualified variables
    bool isFunction;          // True if this is a function
    bool isDefined;           // For tracking function definitions
    vector<string> paramTypes;  // For function parameters
    int arraySize;            // For arrays (-1 if not array)
};

class SymbolTable {
    map<string, SymbolTableEntry> symbols;
    SymbolTable* parent;      // For scope chaining
    
public:
    // Constructor
    SymbolTable(SymbolTable* parentScope = nullptr) : parent(parentScope) {}

    bool insert(const string& name, const SymbolTableEntry& entry) {
        // Check if symbol already exists in current scope
        if (symbols.find(name) != symbols.end()) {
            return false;  // Already exists in current scope
        }
        symbols[name] = entry;
        return true;
    }

    SymbolTableEntry* lookup(const string& name) {
        // Look in current scope
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            return &(it->second);
        }
        
        // If not found and we have a parent scope, look there
        if (parent != nullptr) {
            return parent->lookup(name);
        }
        
        return nullptr;  // Not found in any scope
    }

    SymbolTable* createChildScope() {
        return new SymbolTable(this);
    }

    // Getter for parent scope
    SymbolTable* getParent() const {
        return parent;
    }
};

void reportError(Node* node) {
    // First print the non-terminal (left side of production)
    cout << node->symbol << " ::= ";
    
    // Then print all child nodes (right side of production)
    for (Node* child : node->children) {
        if (!child->content.empty()) {
            // This is a terminal with line number and lexeme
            // Split the content to get the parts
            istringstream iss(child->content);
            string token, line, lexeme;
            iss >> token >> line >> lexeme;
            
            // Print in the format TOKEN(line,lexeme)
            cout << token << "(" << line << "," << lexeme << ")";
            
            // If there are more children after this one, add a space
            if (child != node->children.back()) {
                cout << " ";
            }
        } else {
            // This is a non-terminal (has angle brackets)
            cout << child->symbol;
            
            // If there are more children after this one, add a space
            if (child != node->children.back()) {
                cout << " ";
            }
        }
    }
    cout << endl;
    exit(0);
}




// Function to read indentation level
int getIndentationLevel(const string& line) {
    int spaces = 0;
    while (spaces < line.length() && line[spaces] == ' ') {
        spaces++;
    }
    return spaces;
}

// Function to parse node name from line (extracts content between < >)
string parseNodeName(const string& line) {
    if (line[0] == '<') {
        size_t end = line.find('>');
        if (end != string::npos) {
            return line.substr(0, end + 1);  // Keep the < and >
        }
    }
    return "";
}

// Function to parse content (everything that's not a node declaration)
string parseContent(const string& line) {
    if (line[0] != '<') {
        return line;
    }
    return "";
}

// Recursive function to build the tree
Node* buildTree() {
    string line;
    if (!getline(cin, line)) {
        return nullptr;
    }
    
    // Trim leading spaces while keeping count
    int currentIndentation = getIndentationLevel(line);
    line = line.substr(currentIndentation);
    
    // Check for empty line or end marker
    if (line.empty() || line[0] == '$') {
        return nullptr;
    }

    // Create node based on line content
    Node* currentNode;
    if (line[0] == '<') {
        // This is a symbol node (like <S>, <A>, <B>)
        string symbol = parseNodeName(line);
        currentNode = new Node(symbol);
    } else {
        // This is a content node (like "NIZ_ZNAKOVA 2 "\a\b\c"")
        currentNode = new Node("");
        currentNode->content = line;
        
        // Parse and set lexical unit if this is a terminal
        istringstream iss(line);
        string token, line_num, lexeme;
        iss >> token;  // Get token type (NIZ_ZNAKOVA, etc)
        iss >> line_num;  // Get line number
        
        // Get the rest of the line as lexeme (to handle strings with spaces)
        getline(iss >> ws, lexeme);
        currentNode->lexicalUnit = lexeme;
    }

    // Read next line to peek at indentation
    string nextLine;
    while (getline(cin, line)) {
        int nextIndentation = getIndentationLevel(line);
        
        // If next line has less or equal indentation, we're done with this node
        if (nextIndentation <= currentIndentation) {
            cin.seekg(-line.length() - 1, ios::cur); // Push back the line
            break;
        }
        
        // Process child node
        cin.seekg(-line.length() - 1, ios::cur); // Push back the line
        Node* child = buildTree();
        if (child != nullptr) {
            currentNode->children.push_back(child);
        }
    }

    return currentNode;
}

// Function to print the tree (for verification)
void printTree(Node* root, int level = 0) {
    if (root == nullptr) return;

    string indent(level, ' ');
    
    if (!root->content.empty()) {
        cout << indent << root->content << endl;
    } else {
        cout << indent << "<" << root->symbol << ">" << endl;
    }

    for (Node* child : root->children) {
        printTree(child, level + 1);
    }
}

#pragma endregion Utils






namespace Constants {
    const int INT_MIN = -2147483648;
    const int INT_MAX = 2147483647;
    const int CHAR_MIN = 0;
    const int CHAR_MAX = 255;

    bool isValidIntConstant(int value) {
        return value >= INT_MIN && value <= INT_MAX;
    }

    bool isValidCharConstant(int value) {
        return value >= CHAR_MIN && value <= CHAR_MAX;
    }

    bool isValidEscapeSequence(char c) {
        return c == 't' || c == 'n' || c == '0' || 
               c == '\'' || c == '\"' || c == '\\';
    }
}

class ExpressionValidator {
public:
    static bool validateBrojConstant(const string& lexeme, TypeInfo& outType) {
        try {
            int value = stoi(lexeme);
            if (!Constants::isValidIntConstant(value)) {
                return false;
            }
            outType = TypeInfo(BasicType::INT);
            return true;
        } catch (...) {
            return false;
        }
    }

    static bool validateZnakConstant(const string& lexeme, TypeInfo& outType) {
        string charValue = lexeme.substr(1, lexeme.length() - 2);  // Remove quotes
        
        if (charValue.length() == 1) {
            if (!Constants::isValidCharConstant(static_cast<int>(charValue[0]))) {
                return false;
            }
        }
        else if (charValue.length() == 2 && charValue[0] == '\\') {
            if (!Constants::isValidEscapeSequence(charValue[1])) {
                return false;
            }
        }
        else {
            return false;
        }
        
        outType = TypeInfo(BasicType::CHAR);
        return true;
    }

    static bool validateNizZnakova(const string& lexeme, TypeInfo& outType) {
        for (size_t i = 0; i < lexeme.length(); i++) {
            if (lexeme[i] == '\\') {
                if (i + 1 >= lexeme.length()) {
                    return false;
                }
                if (!Constants::isValidEscapeSequence(lexeme[i + 1])) {
                    return false;
                }
                i++;
            }
            else if (!Constants::isValidCharConstant(static_cast<int>(lexeme[i]))) {
                return false;
            }
        }
        
        outType = TypeInfo::makeArrayType(BasicType::CHAR, true);
        return true;
    }
};

#pragma endregion Types




#pragma region Processors
// Za obradu izraza (4.4.4)
class IzrazProcessor {
public:
    IzrazProcessor(SymbolTable* scope) : currentScope(scope) {}

    void process_primarni_izraz(Node* node);
    // void process_postfiks_izraz(Node* node);
    // void process_lista_argumenata(Node* node);
    // void process_unarni_izraz(Node* node);
    // void process_unarni_operator(Node* node);
    // void process_cast_izraz(Node* node);
    // void process_ime_tipa(Node* node);
    // void process_specifikator_tipa(Node* node);
    // void process_multiplikativni_izraz(Node* node);
    // void process_aditivni_izraz(Node* node);
    // void process_odnosni_izraz(Node* node);
    // void process_jednakosni_izraz(Node* node);
    // void process_bin_i_izraz(Node* node);
    // void process_bin_xili_izraz(Node* node);
    // void process_bin_ili_izraz(Node* node);
    // void process_log_i_izraz(Node* node);
    // void process_log_ili_izraz(Node* node);
    void process_izraz_pridruzivanja(Node* node); // not implemented
    void process_izraz(Node* node);

private:
    SymbolTable* currentScope;  // Add this member variable
};

// Implementation of the public function
void IzrazProcessor::process_primarni_izraz(Node* node) {
    if (node->children.size() != 1 && node->children.size() != 3) {
        reportError(node);
        return;
    }

    if (node->children.size() == 3) {
        if (node->children[0]->content.find("L_ZAGRADA") == 0 &&
            node->children[2]->content.find("D_ZAGRADA") == 0) {
            process_izraz(node->children[1]);
            node->typeInfo = node->children[1]->typeInfo;
            node->isLValue = node->children[1]->isLValue;
            return;
        }
        reportError(node);
        return;
    }

    Node* child = node->children[0];
    string childContent = child->content;
    node->isLValue = false;

    if (childContent.find("IDN") == 0) {
        SymbolTableEntry* entry = currentScope->lookup(child->lexicalUnit);
        if (!entry) {
            reportError(node);
            return;
        }
        
        if (entry->type == "int") {
            node->typeInfo = TypeInfo(BasicType::INT, entry->isConstant);
        } else if (entry->type == "char") {
            node->typeInfo = TypeInfo(BasicType::CHAR, entry->isConstant);
        } else if (entry->type == "void") {
            node->typeInfo = TypeInfo(BasicType::VOID, entry->isConstant);
        }
        
        node->isLValue = !entry->isConstant && !entry->isFunction && 
                        entry->arraySize == -1 && 
                        (entry->type == "int" || entry->type == "char");
    }
    else if (childContent.find("BROJ") == 0) {
        if (!ExpressionValidator::validateBrojConstant(child->lexicalUnit, node->typeInfo)) {
            reportError(node);
        }
    }
    else if (childContent.find("ZNAK") == 0) {
        if (!ExpressionValidator::validateZnakConstant(child->lexicalUnit, node->typeInfo)) {
            reportError(node);
        }
    }
    else if (childContent.find("NIZ_ZNAKOVA") == 0) {
        if (!ExpressionValidator::validateNizZnakova(child->lexicalUnit, node->typeInfo)) {
            reportError(node);
        }
    }
    else {
        reportError(node);
    }
}

void IzrazProcessor::process_izraz_pridruzivanja(Node* node){
    // currently not implemented
}

void IzrazProcessor::process_izraz(Node* node) {
    if (!node) {
        reportError(node);
        return;
    }
    
    // According to 4.4.4, <izraz> can be either:
    // <izraz> ::= <izraz_pridruzivanja>
    // <izraz> ::= <izraz> ZAREZ <izraz_pridruzivanja>
    
    if (node->children.size() == 1) {
        // <izraz> ::= <izraz_pridruzivanja>
        process_izraz_pridruzivanja(node->children[0]);
        node->type = node->children[0]->type;
        node->isLValue = false;  // Comma expressions are never l-values
    }
    else if (node->children.size() == 3) {
        // <izraz> ::= <izraz> ZAREZ <izraz_pridruzivanja>
        process_izraz(node->children[0]);
        process_izraz_pridruzivanja(node->children[2]);
        node->type = node->children[2]->type;  // Type of the last expression
        node->isLValue = false;
    }
    else {
        reportError(node);
    }
}

// Za obradu programske strukture (4.4.5)
class NaredbaProcessor {
public:
    // void process_slozena_naredba(Node* node);
    // void process_lista_naredbi(Node* node);
    // void process_naredba(Node* node);
    // void process_izraz_naredba(Node* node);
    // void process_naredba_grananja(Node* node);
    // void process_naredba_petlje(Node* node);
    // void process_naredba_skoka(Node* node);
};

// Za obradu deklaracija i definicija (4.4.6)
class DeklaracijaProcessor {
public:
    // void process_definicija_funkcije(Node* node);
    // void process_lista_parametara(Node* node);
    // void process_deklaracija_parametra(Node* node);
    // void process_lista_deklaracija(Node* node);
    // void process_deklaracija(Node* node);
    // void process_lista_init_deklaratora(Node* node);
    // void process_init_deklarator(Node* node);
    // void process_izravni_deklarator(Node* node);
    // void process_inicijalizator(Node* node);
};

#pragma region Processors

#pragma region Claude_definitions




class SemanticAnalyzer {
private:
    Node* root;
    SymbolTable* currentScope;
    IzrazProcessor izrazProcessor;  // Add processor as member
    
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
    }

public:
    SemanticAnalyzer(Node* rootNode) : root(rootNode), currentScope(nullptr), izrazProcessor(currentScope) {}

    void analyze() {
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
        else if (node->symbol == "<definicija_funkcije>") {
            // Validate function definition
        }
        else if (node->symbol == "<deklaracija>") {
            // Validate variable/function declarations
        }
        // ... handle other node types
        
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