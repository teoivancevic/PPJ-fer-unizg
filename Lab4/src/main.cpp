#include "FRISCGenerator.hpp"

void printTree(Node *node, int n = 0)
{
    for (int i = 0; i < n; i++)
        cout << " ";
    cerr << node->symbol << "\n";
    for (Node *bby : node->children)
        printTree(bby, n + 1);
}

// main.cpp changes:
int main() {
    // cerr << "Starting cpp program\n";
    Node* root = TreeUtils::buildTree();
    // printTree(root);
    FRISCGenerator generator;
    generator.generate(root);
    delete root;
    return 0;
}