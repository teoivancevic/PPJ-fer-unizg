#include "FRISCGenerator.hpp"

void printTree(Node *node, int n = 0)
{
    for (int i = 0; i < n; i++)
        cout << " ";
    cout << node->symbol << "\n";
    for (Node *bby : node->children)
        printTree(bby, n + 1);
}

// main.cpp changes:
int main() {
    Node* root = TreeUtils::buildTree();
    FRISCGenerator generator;
    generator.generate(root);
    delete root;
    return 0;
}