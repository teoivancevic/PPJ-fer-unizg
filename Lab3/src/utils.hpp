#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm> 

using namespace std;



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
    Node* parent;          // Parent node pointer
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

        // Handle basic type conversion cases
        if (source.getBaseType() == target.getBaseType()) {
            // If source isn't const or target is const, basic types are compatible
            if (!source.isConst() || target.isConst()) {
                return true;
            }
        }

        // Handle char to int conversion (with const preservation)
        if (source.getBaseType() == BasicType::CHAR && 
            target.getBaseType() == BasicType::INT) {
            // For const char to int, target must be const
            if (source.isConst() && !target.isConst()) {
                return false;
            }
            return true;
        }

        // Handle array type conversions
        if (isArrayType(source) && isArrayType(target)) {
            // Arrays must have compatible base types
            if (source.getBaseType() != target.getBaseType()) {
                return false;
            }
            // niz(T) ⇠ niz(const(T)) but not vice versa
            return !source.isConst() || target.isConst();
        }

        // Handle function type compatibility
        if (isFunctionType(source) && isFunctionType(target)) {
            // Return types must be compatible
            if (!areTypesCompatible(
                TypeInfo(source.getReturnType()), 
                TypeInfo(target.getReturnType()))) {
                return false;
            }

            const auto& sourceParams = source.getFunctionParams();
            const auto& targetParams = target.getFunctionParams();
            
            if (sourceParams.size() != targetParams.size()) {
                return false;
            }

            // Parameters must be compatible
            for (size_t i = 0; i < sourceParams.size(); i++) {
                if (!areTypesCompatible(sourceParams[i], targetParams[i])) {
                    return false;
                }
            }
            return true;
        }

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

    const map<string, SymbolTableEntry>& getSymbols() const {
        return symbols;
    }

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
            child->parent = currentNode;  // Set parent pointer
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
            // if (!Constants::isValidIntConstant(value)) {
            //     return false;
            // }
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








#endif // UTILS_HPP