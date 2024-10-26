#include"automata.hpp"

namespace resources 
{
    using State = std::string;
    using ID = uint32_t;

    template<typename T>
    using Container = std::vector<T>;

    static Container<State> STATES;
    static Container<std::string> SYMBOLS;
    static std::map<ID, NKA> AUTOMATA;
    static std::map<State, Container<ID>> TABLE;
    static State START;
    
    void init() 
    {
        START = "S_poc";
        STATES.emplace_back("S_poc");
        STATES.emplace_back("S_preskoci");
        SYMBOLS.emplace_back("A");
        TABLE["S_poc"].push_back(0);
            AUTOMATA[0].link(0, 1, 97);
        AUTOMATA[0].start = 0; AUTOMATA[0].end = 1;
        AUTOMATA[0].name = "A";
        AUTOMATA[0].commands.emplace_back("UDJI_U_STANJE S_preskoci");
        TABLE["S_poc"].push_back(1);
            AUTOMATA[1].link(0, 1, 10);
        AUTOMATA[1].start = 0; AUTOMATA[1].end = 1;
        AUTOMATA[1].name = "-";
        AUTOMATA[1].commands.emplace_back("NOVI_REDAK");
        TABLE["S_preskoci"].push_back(2);
            AUTOMATA[2].link(0, 1, 97);
        AUTOMATA[2].start = 0; AUTOMATA[2].end = 1;
        AUTOMATA[2].name = "-";
        AUTOMATA[2].commands.emplace_back("UDJI_U_STANJE S_poc");
        TABLE["S_preskoci"].push_back(3);
            AUTOMATA[3].link(0, 1, 10);
        AUTOMATA[3].start = 0; AUTOMATA[3].end = 1;
        AUTOMATA[3].name = "-";
        AUTOMATA[3].commands.emplace_back("NOVI_REDAK");
    }   
}
