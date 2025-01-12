#pragma once

#include "Types.hpp"

struct SymbolTable
{
    struct Entry
    {
        TypeInfo type;
        string name;
        bool isDefined = false;
        int arraySize = -1; // For arrays (-1 if not array)
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
    string lineNumber; // For error reporting
    string lexicalUnit;

    int arraySize; // For array declarations

    Node();

    Node(string symbol, Node *parent = nullptr, string content = "")
        : symbol(symbol), parent(parent), content(content)
    {
        if (content != "")
        {
            this->symbol = consumeNextWord(content);
            this->lineNumber = consumeNextWord(content);
            this->lexicalUnit = consumeNextWord(content);
        }
    }

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

namespace TreeUtils
{
    Node *buildTree()
    {
        string line;
        getline(cin, line);

        vector<pair<int, Node *>> stack;

        auto [indent, content] = parseContent(line);
        stack.push_back({indent, new Node(content)});

        while (getline(cin, line) && !line.empty() && !(line == "$"))
        {
            auto [indent, content] = parseContent(line);

            while (indent <= stack.back().first)
                stack.pop_back();

            auto current = stack.back().second;

            if (content[0] != '<')
                current->children.push_back(new Node("", current, content));

            else if (indent > stack.back().first)
            {
                Node *next = new Node(content, current);
                stack.push_back({indent, next});
                current->children.push_back(next);
            }
        }
        return stack[0].second;
    }

    // Function to parse node name and indent number from line
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

    void reportError(Node *node)
    {
        cout << node->symbol << " ::= ";

        for (Node *child : node->children)
        {
            cout << child->symbol;

            if (child->isTerminating())
                cout << "(" << child->lineNumber << "," << child->lexicalUnit << ")";

            if (child != node->children.back())
                cout << " ";
        }

        cout << endl;
        exit(0);
    }
}

using SymbolTableEntry = SymbolTable::Entry;