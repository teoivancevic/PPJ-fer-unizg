#pragma once

#include "Types.hpp"

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
    static Node *buildTree();

    static pair<int, string> parseContent(const string &line);

    static int getIndentationLevel(const string &line);

    static void printTree(Node *root, int level = 0);

    static void reportError(Node *node);
}

struct Node
{
    Node *parent; // Parent node pointer
    vector<Node *> children;

    string symbol; // Node symbol (A, B, etc.)

    TypeInfo typeInfo;
    bool isLValue; // For l-value checking

    string content;
    int lineNumber; // For error reporting
    string lexicalUnit;

    int arraySize; // For array declarations

    Node();

    Node(string symbol, Node *parent = nullptr) : symbol(symbol), parent(parent) {}

    bool isTerminating() { return !(symbol[0] == '<'); }

    bool isNewScope(Node *node) { return node->symbol == "<slozena_naredba>" ||
                                         node->symbol == "<definicija_funkcije>"; }

    bool isDeclaration(Node *node) { return node->symbol == "<deklaracija>" ||
                                            node->symbol == "<definicija_funkcije>"; }

    ~Node()
    {
        for (Node *child : children)
        {
            delete child;
        }
    }
};

inline Node::Node(std::string content)
{
    this->content = content;
    this->symbol = "";
    this->lexicalUnit = "";
    this->lineNumber = 0;
    this->isLValue = false;
}

namespace TreeUtils
{
    Node *buildTree()
    {
        string line;
        vector<pair<int, Node *>> stack;

        auto [indent, content] = parseContent(line);
        stack.push_back({indent, new Node(content)});

        while (getline(cin, line) && !line.empty() && !(line == "$"))
        {
            auto [indent, content] = parseContent(line);
            auto current = stack.back().second;

            if (content[0] != '<')
                forEachWord(content, [current](const string &item)
                            { current->children.push_back(new Node(item, current)); });

            else if (indent > stack.back().first)
            {
                Node *next = new Node(content, current);
                stack.push_back({indent, next});
                current->children.push_back(next);
            }

            while (indent <= stack.back().first)
                stack.pop_back();
        }
        return stack[0].second;
    }

    // Function to parse node name from line (extracts content between < >)
    pair<int, string> parseContent(const string &line)
    {
        int i = 0;
        while (line[i] == ' ')
            i++;

        return {i, line.substr(i, line.size() - i)};
    }

    // Function to read indentation level
    int getIndentationLevel(const string &line)
    {
        int spaces = 0;
        while (spaces < line.length() && line[spaces] == ' ')
            spaces++;
        return spaces;
    }

    // // Function to print the tree (for verification)
    // void printTree(Node *root, int level)
    // {
    //     if (root == nullptr)
    //         return;

    //     string indent(level, ' ');

    //     if (!root->content.empty())
    //     {
    //         cout << indent << root->content << endl;
    //     }
    //     else
    //     {
    //         cout << indent << "<" << root->symbol << ">" << endl;
    //     }

    //     for (Node *child : root->children)
    //     {
    //         printTree(child, level + 1);
    //     }
    // }

    void reportError(Node *node)
    {
        // First print the non-terminal (left side of production)
        cout << node->symbol << " ::= ";

        // Then print all child nodes (right side of production)
        // for (Node *child : node->children)
        // {
        //     if (!child->content.empty())
        //     {
        //         // // This is a terminal with line number and lexeme
        //         // // Split the content to get the parts
        //         // istringstream iss(child->content);
        //         // string token, line, lexeme;
        //         // iss >> token >> line >> lexeme;

        //         // // Print in the format TOKEN(line,lexeme)
        //         // cout << token << "(" << line << "," << lexeme << ")";

        //         // // If there are more children after this one, add a space
        //         // if (child != node->children.back())
        //         // {
        //         //     cout << " ";
        //         // }
        //     }
        //     else
        //     {
        //         // This is a non-terminal (has angle brackets)
        //         cout << child->symbol;

        //         // If there are more children after this one, add a space
        //         if (child != node->children.back())
        //         {
        //             cout << " ";
        //         }
        //     }
        // }
        cout << endl;
        exit(0);
    }
}

using SymbolTableEntry = SymbolTable::Entry;

#endif