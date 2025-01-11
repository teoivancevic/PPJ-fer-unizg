#include "Types.hpp"

#pragma region Table

// look into it
struct SymbolTable
{
    struct Entry
    {
        string name;
        TypeInfo type;  // Basic type or function signature, // "int", "char", "void", or function type
        bool isDefined; // For tracking definitions
        int arraySize;  // For arrays (-1 if not array)
    };

    map<string, Entry> symbols;
    SymbolTable *parent; // For scope chaining

    // Constructor
    SymbolTable(SymbolTable *parentScope = nullptr) : parent(parentScope) {}

    bool insert(const string &name, const Entry &entry)
    {
        // Check if symbol already exists in current scope
        if (exists(symbols, name))
            return false;
        symbols.insert({name, entry});
        return true;
    }

    Entry *lookup(const string &name)
    {
        // Look in current scope
        if (exists(symbols, name))
            return &symbols.at(name);

        // If not found and we have a parent scope, look there
        if (parent != nullptr)
            return parent->lookup(name);

        return nullptr; // Not found in any scope
    }

    SymbolTable *createChildScope()
    {
        return new SymbolTable(this);
    }
};

using UnitTable = SymbolTable;

#pragma endregion Table

#pragma region Tree
// Tree node structure
struct Node
{
    string symbol; // Node symbol (A, B, etc.)
    vector<Node *> children;
    string content; // Content within the node (a1xxx, b2yy, etc.)

    // New semantic-related members
    TypeInfo typeInfo; // For tracking types (int, char, void, etc.)
    bool isLValue;     // For l-value checking

    // for error reporting
    int lineNumber;
    string lexicalUnit;

    int arraySize; // For array declarations

    template <int N>
    Node(const array<string, N> &s);

    Node();

    Node(string);

    ~Node()
    {
        for (Node *child : children)
        {
            delete child;
        }
    }
};

void reportError(Node *node)
{
    // First print the non-terminal (left side of production)
    cout << node->symbol << " ::= ";

    // Then print all child nodes (right side of production)
    for (Node *child : node->children)
    {
        if (!child->content.empty())
        {
            // This is a terminal with line number and lexeme
            // Split the content to get the parts
            istringstream iss(child->content);
            string token, line, lexeme;
            iss >> token >> line >> lexeme;

            // Print in the format TOKEN(line,lexeme)
            cout << token << "(" << line << "," << lexeme << ")";

            // If there are more children after this one, add a space
            if (child != node->children.back())
            {
                cout << " ";
            }
        }
        else
        {
            // This is a non-terminal (has angle brackets)
            cout << child->symbol;

            // If there are more children after this one, add a space
            if (child != node->children.back())
            {
                cout << " ";
            }
        }
    }
    cout << endl;
    exit(0);
}

// Function to parse node name from line (extracts content between < >)
string parseNodeName(const string &line)
{
    if (line[0] == '<')
    {
        size_t end = line.find('>');
        if (end != string::npos)
        {
            return line.substr(0, end + 1); // Keep the < and >
        }
    }
    return "";
}

// Function to parse content (everything that's not a node declaration)
static inline string parseContent(const string &line)
{
    return line[0] != '<' ? line : "";
}

// Function to read indentation level
int getIndentationLevel(const string &line)
{
    int spaces = 0;
    while (spaces < line.length() && line[spaces] == ' ')
        spaces++;
    return spaces;
}

// Recursive function to build the tree
Node *buildTree()
{
    string line;
    if (!getline(cin, line))
    {
        return nullptr;
    }

    // Trim leading spaces while keeping count
    int currentIndentation = getIndentationLevel(line);
    line = line.substr(currentIndentation);

    // Check for empty line or end marker
    if (line.empty() || line[0] == '$')
    {
        return nullptr;
    }

    // Create node based on line content
    Node *currentNode;
    if (line[0] == '<')
    {
        // This is a symbol node (like <S>, <A>, <B>)
        string symbol = parseNodeName(line);
        currentNode = new Node(symbol);
    }
    else
    {
        // This is a content node (like "NIZ_ZNAKOVA 2 "\a\b\c"")
        currentNode = new Node("");
        currentNode->content = line;

        // Parse and set lexical unit if this is a terminal
        istringstream iss(line);
        string token, line_num, lexeme;
        iss >> token;    // Get token type (NIZ_ZNAKOVA, etc)
        iss >> line_num; // Get line number

        // Get the rest of the line as lexeme (to handle strings with spaces)
        getline(iss >> ws, lexeme);
        currentNode->lexicalUnit = lexeme;
    }

    // Read next line to peek at indentation
    string nextLine;
    while (getline(cin, line))
    {
        int nextIndentation = getIndentationLevel(line);

        // If next line has less or equal indentation, we're done with this node
        if (nextIndentation <= currentIndentation)
        {
            cin.seekg(-line.length() - 1, ios::cur); // Push back the line
            break;
        }

        // Process child node
        cin.seekg(-line.length() - 1, ios::cur); // Push back the line
        Node *child = buildTree();
        if (child != nullptr)
        {
            currentNode->children.push_back(child);
        }
    }

    return currentNode;
}

// Function to print the tree (for verification)
void printTree(Node *root, int level = 0)
{
    if (root == nullptr)
        return;

    string indent(level, ' ');

    if (!root->content.empty())
    {
        cout << indent << root->content << endl;
    }
    else
    {
        cout << indent << "<" << root->symbol << ">" << endl;
    }

    for (Node *child : root->children)
    {
        printTree(child, level + 1);
    }
}

#pragma endregion Tree