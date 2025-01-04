#include <fstream>
#include <utils.hpp>
#include <actions.hpp>

class Syntax
{

    template <ptr... args>
    Node<args...> makeNode(sym s)
    {
    }

public:
    void consume(string input);

private:
};