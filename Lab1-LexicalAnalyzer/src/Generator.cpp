#define REGEX_INITIALIZABLE
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "filegen_defs.hpp"
#include "Utils.hpp"
#include "Regex.hpp"
#include "automata.hpp"

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

class Generator 
{
    using State = std::string;
    using ID = uint32_t;

    template<typename T>
    using Container = std::vector<T>;

    enum ReadState {
        HEADER,
        BODY
    };

    std::ifstream in;
    std::ofstream out;
    bool read_stdin = false;
    bool write_stdout = false;

public:

    Generator(const std::string& inputStream = "cin", const std::string& outStream = "") 
    {
        if (inputStream != "cin") 
            in = std::ifstream(inputStream);
        else 
            write_stdout = true;
        
        if (outStream.empty()) 
            out = std::ofstream(file_no_extension(inputStream) + ".hpp");
        else if (outStream != "cout") 
            out = std::ofstream(outStream);
        else 
            read_stdin = true;
    }

    void generate() 
    {
        std::string line;
        State state;
        ReadState phase = HEADER;
        int id = -1;
        bool first = true;

        #define GEN_IN (read_stdin ? std::cin : in), line
        #define GEN_OUT (write_stdout ? std::cout : this->out)
        
        GEN_OUT <<CPP_BEGIN <<std::endl;

        if (read_stdin || in.is_open()) 
        {
            std::string fst;
            while (getline(GEN_IN))
            {
                if (line.empty()) return;

                fst = consumeNextWord(line);
                
                if (fst[0] == '{')
                    Regex::saved[fst] = line;

                else if (fst[0] == '%')
                    consumeEachWord(line, [this, fst, &first](std::string&& word) mutable {
                        if (first) {
                            GEN_OUT <<indent <<set_start(word.c_str()) <<std::endl;
                            first = false;
                        }
                        GEN_OUT <<indent <<(fst[1] == 'X' ? add_state(word.c_str()) : add_symbol(word.c_str())) <<std::endl;
                    });

                if (fst[1] == 'L') break;
            }

            while (getline(GEN_IN)) 
            {
                if (line.empty()) return;
                
                if (phase == BODY) {
                    fst = readNextWord(line);
                    if (fst[0] == '}') phase = HEADER;
                    else out <<indent <<add_command(id, line.c_str()) <<std::endl;
                } else {
                    fst = consumeNextWord(line, '>');
                    state = fst.substr(1, fst.size()-1);
                    GEN_OUT <<indent <<add_automata(state.c_str(), ++id) <<std::endl;
                    NKA nka = consumeNextWord(line);
                    for (ID id1 = 0; id1 < nka.size(); id1++) 
                        for (char s : nka.get_transition_symbols(id1))
                            for (ID id2 : nka.get_transitions(id1, s))
                                if (id1 != id2) GEN_OUT <<indent2 <<link_state(id, id1, id2, s) <<std::endl;
                    GEN_OUT <<indent <<set_start_end(id, nka.start, nka.end) <<std::endl;
                    getline(GEN_IN); getline(GEN_IN);
                    GEN_OUT <<indent <<set_name(id, line.c_str()) <<std::endl;
                    phase = BODY;
                }
            }

            GEN_OUT <<CPP_END <<std::endl;

            if (!read_stdin) in.close();
            if (!write_stdout) out.close();
        }
        else 
            std::cerr << "Unable to open file or stream!" << "\n";
    }
};

int main () 
{
    // std::string file;
    // std::cin >>file;
    Generator("input/simplePpjLang.lan", "analizator/table.hpp").generate();
}