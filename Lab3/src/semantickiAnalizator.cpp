#include "Processors.hpp"

int main()
{
    Node *root = TreeUtils::buildTree();

    // Print the tree to verify the structure
    if (root != nullptr)
    {
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