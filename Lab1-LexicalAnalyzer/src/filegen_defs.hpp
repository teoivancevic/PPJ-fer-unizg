static const std::string CPP_BEGIN = 
R"a(#include"automata.hpp"

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
    {)a";

#define indent "        "
#define indent2 "            "
#define add_command(id, line) string_format("AUTOMATA[%d].commands.emplace_back(\"%s\");", id, line)
#define add_automata(state, id) string_format("TABLE[\"%s\"].push_back(%d);", state, id)
#define link_state(id, s1, s2, sym) string_format("AUTOMATA[%d].link(%d, %d, %d);", id, s1, s2, sym)
#define set_name(id, name) string_format("AUTOMATA[%d].name = \"%s\";", id, name)
#define set_start(state) string_format("START = \"%s\";", state)
#define add_state(state) string_format("STATES.emplace_back(\"%s\");", state)
#define add_symbol(symbol) string_format("SYMBOLS.emplace_back(\"%s\");", symbol)
#define set_start_end(id, start, end) string_format("AUTOMATA[%d].start = %d; AUTOMATA[%d].end = %d;", id, start, id, end)

static const std::string CPP_END = 
R"b(    }   
})b";

static std::string convert_to_raw (const std::string& str) {
    std::string rez = "";
    for (char c : str) {
        if (c == '\"' || c == '\\' || c == '\'') rez += '\\';
        rez += c;
    }
    return rez;
}