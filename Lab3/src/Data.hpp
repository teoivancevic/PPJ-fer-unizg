#include "Types.hpp"
#include <fstream>

struct SymbolTable
{
    struct Entry
    {
        // Zašto ne koristiti TypeInfo?
        string type;
        bool isConstant = false;
        bool isFunction = false;
        bool isDefined = false;
        vector<string> paramTypes;

        string name;
        int arraySize = -1; // For arrays (-1 if not array)
    };

    Entry makeEntry(const TypeInfo &info)
    {
        Entry e;

        e.type = TypeUtils::typeToString(info.getBaseType());
        e.isConstant = info.isConst();
        e.isFunction = info.isFunc();
        e.arraySize = info.isArray() ? 0 : -1;

        return e;
    }

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

    bool insert(const string &name, const TypeInfo &info) { return insert(name, makeEntry(info)); }

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

    SymbolTable *createChildScope() { return new SymbolTable(this); }
};

struct Node;

namespace TreeUtils
{
    Node *buildTree();
    string parseNodeName(const string &line);
    static inline string parseContent(const string &line);
    int getIndentationLevel(const string &line);
    void printTree(Node *root, int level = 0);
    void reportError(Node *node);
}

// Tree node structure
struct Node
{
    string symbol; // Node symbol (A, B, etc.)
    vector<Node *> children;
    Node *parent;   // Parent node pointer
    string content; // Content within the node (a1xxx, b2yy, etc.)

    // New semantic-related members
    string type; // For tracking types (int, char, void, etc.)
    TypeInfo typeInfo;
    bool isLValue; // For l-value checking

    int lineNumber;     // For error reporting
    string lexicalUnit; // For error reporting

    int arraySize;             // For array declarations
    vector<string> paramTypes; // vec u typeinfo

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

namespace TreeUtils
{
    Node *buildTree()
    {
        string line;
        if (!getline(cin, line))
            return nullptr;

        int currentIndentation = getIndentationLevel(line);
        line = line.substr(currentIndentation);

        if (line.empty() || line[0] == '$')
            return nullptr;

        Node *currentNode;
        if (line[0] == '<')
            // symbol node
            currentNode = new Node(parseNodeName(line));
        else
        { // content node
          // currentNode = new Node("");
          // currentNode->content = line;

            // // Parse and set lexical unit if this is a terminal
            // istringstream iss(line);
            // string token, line_num, lexeme;
            // iss >> token;    // Get token type (NIZ_ZNAKOVA, etc)
            // iss >> line_num; // Get line number

            // // Get the rest of the line as lexeme (to handle strings with spaces)
            // getline(iss >> ws, lexeme);
            // currentNode->lexicalUnit = lexeme;
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
                child->parent = currentNode; // Set parent pointer
                currentNode->children.push_back(child);
            }
        }

        return currentNode;
    }

    // Function to parse node name from line (extracts content between < >)
    string parseNodeName(const string &line)
    {
        if (line[0] == '<')
        {
            size_t end = line.find('>');
            if (end != string::npos)
                return line.substr(0, end + 1); // Keep the < and >
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

    // Function to print the tree (for verification)
    void printTree(Node *root, int level)
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

    void reportError(Node *node)
    {
        // First print the non-terminal (left side of production)
        cout << node->symbol << " ::= ";

        // Then print all child nodes (right side of production)
        for (Node *child : node->children)
        {
            if (!child->content.empty())
            {
                // // This is a terminal with line number and lexeme
                // // Split the content to get the parts
                // istringstream iss(child->content);
                // string token, line, lexeme;
                // iss >> token >> line >> lexeme;

                // // Print in the format TOKEN(line,lexeme)
                // cout << token << "(" << line << "," << lexeme << ")";

                // // If there are more children after this one, add a space
                // if (child != node->children.back())
                // {
                //     cout << " ";
                // }
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

}

using SymbolTableEntry = SymbolTable::Entry;