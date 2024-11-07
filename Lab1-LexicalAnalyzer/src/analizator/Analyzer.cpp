#if defined __has_include
#if __has_include ("table.txt")

#include"table.hpp"
#include<iostream>
#include<fstream>
#include<stdexcept>

using namespace resources;

using std::cin;
using std::cout;

class Analyzer 
{
    int it = 0;
    int lastFound;
    int lastRead = -1;
    int errorAt = -1;
    int errorStart;
    
    int rule_f = false;
    ID rule;

    size_t size;
    const char* input;

    int rowCounter = 1;
    bool rowCounter_u = false;
    State state = START;

    enum ErrorType {
        UNKNOWN,
        UNKNOWN_EXPRESSION,
        UNKNOWN_COMMAND
    };

public:

    Analyzer (const std::string& exp) : size(exp.size()), input(exp.c_str()) {}

    int row() {
        return rowCounter - rowCounter_u;
    }

    void analyze() 
    {
        for (; it < size; it++) 
        {
            bool found = false, empty = true;
            char sym = input[it];

            for (ID id : TABLE[state]) {
                if (!AUTOMATA[id].empty()) {
                    if (AUTOMATA[id].push_sym(sym)) {
                        if (!found) {
                            rule_f = true;
                            rule = id;
                            found = true;
                        } else rule = std::min(rule, id);
                    }
                    empty &= AUTOMATA[id].empty();
                }
            } 

            if (found) lastFound = it;

            if (empty) 
            {
                if (!rule_f) {
                    if (errorAt != row()) {
                        errorStart = lastRead + 1;
                        errorAt = row();
                    }
                    lastRead++;
                    it = lastRead;
                } 
                else 
                {
                    if (errorAt != -1) {
                        error(UNKNOWN_EXPRESSION, get_exp(errorStart, lastRead).c_str(), errorAt);
                        errorAt = -1;
                    }
                    
                    it = lastFound;

                    run(rule);
                    store(rule);

                    lastRead = it;
                    rule_f = false;
                    rowCounter_u = false;
                }
                
                for (ID id : TABLE[state]) 
                    AUTOMATA[id].reset();
            }
        }
    }

private:

    void run (ID id) 
    {
        for (const auto& command : AUTOMATA[id].commands) 
        {
            std::string com = readNextWord(command);

            if (com == "NOVI_REDAK") 
                rowCounter_u = ++rowCounter;
            else if (com == "UDJI_U_STANJE")
                state = readNextWord(command, 14);
            else if (com == "VRATI_SE") 
                it = lastRead + to_int(readNextWord(command, 9));
            else 
                error(UNKNOWN_COMMAND, com.c_str(), id);
        }
    }

    void store (ID id) 
    {
        if (AUTOMATA[id].name != "-")
            cout <<AUTOMATA[id].name <<" " <<row() <<" " <<get_exp(lastRead + 1, it) <<std::endl;
    }

    std::string get_exp (int start, int end) {
        std::string exp = "";
        while (start <= end) exp += input[start++];
        return exp;
    }

    template <typename ...Args>
    void error(ErrorType err, Args... args) 
    {
        if (err == UNKNOWN_EXPRESSION)
            std::cerr << string_format("Unknown expression: \"%s\" in line %d", args...) <<std::endl;
        else if (err == UNKNOWN_COMMAND) 
            throw std::invalid_argument(string_format("Unknown command: \"%s\" in rule number: \"%d\"", args...));
        else 
            throw std::invalid_argument("Unknown Exception Has occured!");
    }
};

int main () 
{
    resources::init();

    //std::ifstream file = std::ifstream("input/state_hopper.in");

    std::string input = "", line;
    // while (std::getline(file, line)) input += line + "\n";
    while (std::getline(std::cin, line)) input += line + "\n";

    Analyzer(input).analyze();
}
#endif
#endif