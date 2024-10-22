
#include"automata.hpp"

namespace resources {
    using State = std::string;
    using ID = uint32_t;

    template<typename T>
    using Container = std::vector<T>;

    static Container<State> STATES;
    static Container<std::string> SYMBOLS;
    static std::map<ID, NKA> AUTOMATA;
    static std::map<State, Container<ID>> TABLE;
    static State START;
    
    void init() {
        STATES.emplace_back("S_pocetno");
        STATES.emplace_back("S_komentar");
        STATES.emplace_back("S_unarni");
        SYMBOLS.emplace_back("OPERAND");
        SYMBOLS.emplace_back("OP_MINUS");
        SYMBOLS.emplace_back("UMINUS");
        SYMBOLS.emplace_back("LIJEVA_ZAGRADA");
        SYMBOLS.emplace_back("DESNA_ZAGRADA");
        TABLE["S_pocetno"].push_back(0);
        AUTOMATA[0] = "\\t|\\_";
        AUTOMATA[0].name = "-";
        TABLE["S_pocetno"].push_back(1);
        AUTOMATA[1] = "\\n";
        AUTOMATA[1].name = "-";
        AUTOMATA[1].commands.emplace_back("NOVI_REDAK");
        TABLE["S_pocetno"].push_back(2);
        AUTOMATA[2] = "#\\|";
        AUTOMATA[2].name = "-";
        AUTOMATA[2].commands.emplace_back("UDJI_U_STANJE S_komentar");
        TABLE["S_komentar"].push_back(3);
        AUTOMATA[3] = "\\|#";
        AUTOMATA[3].name = "-";
        AUTOMATA[3].commands.emplace_back("UDJI_U_STANJE S_pocetno");
        TABLE["S_komentar"].push_back(4);
        AUTOMATA[4] = "\\n";
        AUTOMATA[4].name = "-";
        AUTOMATA[4].commands.emplace_back("NOVI_REDAK");
        TABLE["S_komentar"].push_back(5);
        AUTOMATA[5] = "\\(|\\)|\\{|\\}|\\||\\*|\\\\|\\$|\\t|\\n|\\_|!|\"|#|%|&|\'|+|,|-|.|/|0|1|2|3|4|5|6|7|8|9|:|;|<|=|>|?|@|A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z|[|]|^|_|`|a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z|~";
        AUTOMATA[5].name = "-";
        TABLE["S_pocetno"].push_back(6);
        AUTOMATA[6] = "(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)*|0x(0|1|2|3|4|5|6|7|8|9|a|b|c|d|e|f|A|B|C|D|E|F)(0|1|2|3|4|5|6|7|8|9|a|b|c|d|e|f|A|B|C|D|E|F)*";
        AUTOMATA[6].name = "OPERAND";
        TABLE["S_pocetno"].push_back(7);
        AUTOMATA[7] = "\\(";
        AUTOMATA[7].name = "LIJEVA_ZAGRADA";
        TABLE["S_pocetno"].push_back(8);
        AUTOMATA[8] = "\\)";
        AUTOMATA[8].name = "DESNA_ZAGRADA";
        TABLE["S_pocetno"].push_back(9);
        AUTOMATA[9] = "-";
        AUTOMATA[9].name = "OP_MINUS";
        TABLE["S_pocetno"].push_back(10);
        AUTOMATA[10] = "-(\\t|\\n|\\_)*-";
        AUTOMATA[10].name = "OP_MINUS";
        AUTOMATA[10].commands.emplace_back("UDJI_U_STANJE S_unarni");
        AUTOMATA[10].commands.emplace_back("VRATI_SE 1");
        TABLE["S_pocetno"].push_back(11);
        AUTOMATA[11] = "\\((\\t|\\n|\\_)*-";
        AUTOMATA[11].name = "LIJEVA_ZAGRADA";
        AUTOMATA[11].commands.emplace_back("UDJI_U_STANJE S_unarni");
        AUTOMATA[11].commands.emplace_back("VRATI_SE 1");
        TABLE["S_unarni"].push_back(12);
        AUTOMATA[12] = "\\t|\\_";
        AUTOMATA[12].name = "-";
        TABLE["S_unarni"].push_back(13);
        AUTOMATA[13] = "\\n";
        AUTOMATA[13].name = "-";
        AUTOMATA[13].commands.emplace_back("NOVI_REDAK");
        TABLE["S_unarni"].push_back(14);
        AUTOMATA[14] = "-";
        AUTOMATA[14].name = "UMINUS";
        AUTOMATA[14].commands.emplace_back("UDJI_U_STANJE S_pocetno");
        TABLE["S_unarni"].push_back(15);
        AUTOMATA[15] = "-(\\t|\\n|\\_)*-";
        AUTOMATA[15].name = "UMINUS";
        AUTOMATA[15].commands.emplace_back("VRATI_SE 1");
    }   
}

