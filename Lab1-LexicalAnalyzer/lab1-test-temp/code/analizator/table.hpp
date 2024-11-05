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
        START = "S_prvi";
        STATES.emplace_back("S_prvi");
        STATES.emplace_back("S_drugi");
        STATES.emplace_back("S_treci");
        SYMBOLS.emplace_back("X");
        TABLE["S_prvi"].push_back(0);
            AUTOMATA[0].link(0, 1, 120);
        AUTOMATA[0].start = 0; AUTOMATA[0].end = 1;
        AUTOMATA[0].name = "-";
        AUTOMATA[0].commands.emplace_back("UDJI_U_STANJE S_drugi");
        TABLE["S_drugi"].push_back(1);
            AUTOMATA[1].link(0, 1, 120);
        AUTOMATA[1].start = 0; AUTOMATA[1].end = 1;
        AUTOMATA[1].name = "-";
        AUTOMATA[1].commands.emplace_back("UDJI_U_STANJE S_treci");
        TABLE["S_treci"].push_back(2);
            AUTOMATA[2].link(0, 1, 120);
        AUTOMATA[2].start = 0; AUTOMATA[2].end = 1;
        AUTOMATA[2].name = "X";
        AUTOMATA[2].commands.emplace_back("UDJI_U_STANJE S_prvi");
        TABLE["S_prvi"].push_back(3);
            AUTOMATA[3].link(0, 1, 10);
        AUTOMATA[3].start = 0; AUTOMATA[3].end = 1;
        AUTOMATA[3].name = "-";
        AUTOMATA[3].commands.emplace_back("NOVI_REDAK");
        TABLE["S_drugi"].push_back(4);
            AUTOMATA[4].link(0, 1, 10);
        AUTOMATA[4].start = 0; AUTOMATA[4].end = 1;
        AUTOMATA[4].name = "-";
        AUTOMATA[4].commands.emplace_back("NOVI_REDAK");
        TABLE["S_treci"].push_back(5);
            AUTOMATA[5].link(0, 1, 10);
        AUTOMATA[5].start = 0; AUTOMATA[5].end = 1;
        AUTOMATA[5].name = "-";
        AUTOMATA[5].commands.emplace_back("NOVI_REDAK");
    }   
}
