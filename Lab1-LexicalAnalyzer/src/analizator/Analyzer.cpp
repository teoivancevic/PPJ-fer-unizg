#if defined __has_include
#if __has_include ("table.hpp")

#include"table.hpp"
#include<iostream>
#include<fstream>
#include<stdexcept>

using namespace resources;

class Analyzer 
{
    int it = 0;
    int lastFound = 0;
    int lastRead = 0;
    
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
            char sym = input[it];
            bool found = false, empty = true;

            for (ID id : TABLE[state]) {
                if (!AUTOMATA[id].empty()) {
                    if (AUTOMATA[id].push_sym(sym)) {
                        if (!found) {
                            rule_f = true;
                            rule = id;
                            found = true;
                        } else rule = std::min(rule, id);
                    }
                    empty = false;
                }
            }
            
            if (found) lastFound = it;
            
            if (empty) 
            {
                it = lastFound;
                
                for (ID id : TABLE[state]) 
                    AUTOMATA[id].reset();
                
                run(rule);
                store(rule);
                
                lastRead = it;
                rule_f = false;
                rowCounter_u = false;
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
                error(UNKNOWN_COMMAND, com.c_str());
        }
    }

    void store (ID id) 
    {
        if (!rule_f) 
            error(UNKNOWN_EXPRESSION, row());

        if (AUTOMATA[id].name != "-") {
            std::cout <<AUTOMATA[id].name <<" " <<row() <<" ";
            print(lastRead + 1, it);
            std::cout <<std::endl;
        }
    }

    void print (int start, int end) {
        while (start <= end) std::cout <<input[start++];
    }

    template <typename ...Args>
    void error(ErrorType err, Args... args) 
    {
        if (err == UNKNOWN_EXPRESSION)
            std::cerr << string_format("Unknown expression in line %d", args...) <<std::endl;
        else if (err == UNKNOWN_COMMAND) 
            throw std::invalid_argument(string_format("Unknown command: %s", args...));
        else 
            throw std::invalid_argument("Unknown Exception Has occured!");
    }
};

using std::cin;
using std::cout;

int main () 
{
    resources::init();

    std::ifstream file = std::ifstream("input.txt");

    std::string input = "", line;
    while (std::getline(file, line)) input += line + "\n";

    Analyzer(input).analyze();
}
#endif
#endif