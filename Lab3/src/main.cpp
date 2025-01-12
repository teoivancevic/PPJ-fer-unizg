#include "SemanticAnalyzer.hpp"

void printTree(Node *node, int n = 0)
{
    for (int i = 0; i < n; i++)
        cout << " ";
    cout << node->symbol << "\n";
    for (Node *bby : node->children)
        printTree(bby, n + 1);
}

int main()
{
    Node *root = TreeUtils::buildTree();

    // printTree(root);

    SemanticAnalyzer(root).run();

    delete root;
}