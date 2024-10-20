#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "Regex.hpp"

/* kako koristiti:
    samo stvori prazan generator ako želiš standard ulaz
    inače ga stvori s direktorijem dateoteke koju čitaš, relativno na src direktorij.
    generate() iz ulaza stvara mapu state -> [rule] pod imenom "automata" 
    gdje je state string i odgovara stanju analizatora, a [rule] je niz pravila lex analizatora 
    koja odgovaraju pripadnom stanju. 
    Rule struct sadrži redom regex, ime lex jedinke, i opcionalni niz posebnih naredbi kao u opisu labosa.
    (da bi iz string regexa dobio automat, samo ga proslijedi kao argument NKA klasi)
    Niz stanja analizatora i imena lex jedinki također su dostupna pod imenom "states" i "lex" 
*/

class Generator {
public:
    struct Entry {
        std::string regex;
        std::string name;
        std::vector<std::string> args;
    };

    using State = std::string;

    template<typename T>
    using Container = std::vector<T>;

    std::unordered_map<State, Container<Entry>> automata;
    Container<State> states;
    Container<std::string> lex;

private:

    enum ReadState {
        HEADER,
        BODY
    };

    std::ifstream file;
    bool read_stdin = false;

public:

    Generator() {
        read_stdin = true;
    }

    Generator(const std::string& inputStream) {
        file = std::ifstream(inputStream);
    }

    void generate() 
    {
        std::string line;

        #define GEN_IN_L read_stdin ? std::cin : file, line

        if (read_stdin || file.is_open()) 
        {
            std::string fst;
            while (getline(GEN_IN_L))
            {
                if (line.empty()) return;

                fst = consumeNextWord(line);
                
                if (fst[0] == '{')
                    Regex::saved[fst] = line;

                else if (fst[0] == '%')
                    consumeEachWord(line, [this, fst](std::string&& word) {
                        auto& c = fst[1] == 'X' ? this->states : this->lex;
                        c.emplace_back(word);
                    });

                if (fst[1] == 'L') break;
            }

            State state;
            Entry* atm;
            ReadState phase = HEADER;

            while (getline(GEN_IN_L)) 
            {
                if (line.empty()) return;
                
                if (phase == BODY) {
                    fst = readNextWord(line);
                    if (fst[0] == '}') phase = HEADER;
                    else atm->args.emplace_back(line);
                } else {
                    fst = consumeNextWord(line, '>');
                    state = fst.substr(1, fst.size()-1);
                    automata[state].emplace_back();
                    atm = &automata[state].back();
                    atm->regex = Regex(consumeNextWord(line));
                    getline(GEN_IN_L); getline(GEN_IN_L);
                    atm->name = line;
                    phase = BODY;
                }
            }
            if (!read_stdin) file.close();
        }
        else 
            std::cerr << "Unable to open file or stream!" << "\n";
    }
};