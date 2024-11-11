#pragma once

#include "Utils.hpp"
#include <fstream>

//Dodo sam neka preimenovanja jer mi nisu bila konzistentna
//States bi trebala biti stanja automata, a Symbols znakovi gramatike, drzimo se toga

class Grammar
{
    std::ifstream in;
    bool read_stdin = false;
    bool write_stdout = false;
private:
    vector<std::string> fileLines_backup;
public:

    Symbol BEGIN_SYMBOL;
    set<Symbol> NEZAVRSNI;
    set<Symbol> ZAVRSNI;
    set<Symbol> SYNC_ZAVRSNI;
    map<Symbol, vector<Word>> PRODUKCIJE;

    Grammar(const std::string& inputStream = "cin") 
    {
        if(inputStream != "cin") 
            in = std::ifstream(inputStream);
        else 
            read_stdin = true;
        // std::ifstream file(filePath);
        std::string line;
        #define GEN_IN (read_stdin ? std::cin : in), line

        if (read_stdin || in.is_open())
        {
            Symbol currSymbol = "";
            while(getline(GEN_IN))
            {
                if(line[0] == '%')
                {
                    if(line[1] == 'V')
                        BEGIN_SYMBOL = readSymbol(line, NEZAVRSNI);
                    else if(line[1] == 'T') 
                        readSymbol(line, ZAVRSNI); 
                    else if(line.substr(1, 3) == "Syn") 
                        readSymbol(line, SYNC_ZAVRSNI, 5);
                    else 
                        cerr << "Error in file\n";          
                }
                else if(line[0] == '<')
                {
                    currSymbol = line;
                }
                else if(line[0] == ' ')
                {
                    Word production = {};
                    Symbol symbolsString = line.substr(1, line.size()-1);

                    Symbol symbol = "";
                    for (auto c: symbolsString) {
                        if (c == ' ') {
                            production.emplace_back(symbol);
                            symbol = "";
                        } else {
                            symbol += c;
                        }
                    }
                    production.emplace_back(symbol);

                    PRODUKCIJE[currSymbol].emplace_back(production);
                }
                else
                {
                    cerr << "Error in file\n";
                }

                fileLines_backup.emplace_back(line);
            }
            in.close();
        } 
        else
            cerr << "Unable to open file\n";
    }

    Symbol readSymbol (const std::string line, set<Symbol>& container, int removeFirst = 3) 
    {
        Symbol symbolsString = line.substr(removeFirst, line.size()-removeFirst);
        Symbol firstSymbol = "";
        Symbol symbol = "";
        for (auto c: symbolsString) {
            if (c == ' ') {
                container.emplace(symbol);
                if (firstSymbol == "") firstSymbol = symbol;
                symbol = "";
            } else {
                symbol += c;
            }
        }
        container.emplace(symbol);

        return firstSymbol;
    }

    bool startsWith (const Symbol& sym1, const Symbol& sym2) const 
    {
        if (!PRODUKCIJE.count(sym1)) 
            return sym2 == end_sym ? ZAVRSNI.count(sym1) : false;

        for (const Word& produkcija : PRODUKCIJE.at(sym1))
            if (startsWith(produkcija, sym2)) 
                return true;
        
        return false;
    }

    bool startsWith (const Word& word, const Symbol& sym2) const 
    {
        for (const Symbol& sym1 : word) 
        {
            if (startsWith(sym1, sym2)) {
                if (sym2 == end_sym) continue;
                else return true;
            }
            if (!isVanishing(sym1)) {
                if (sym2 == end_sym) return false;
                else break;
            }
        }

        return sym2 == end_sym;
    }

    bool isVanishing (const Word& word) const {
        return startsWith(word, end_sym);
    }
    bool isVanishing (const Symbol& sym) const {
        return startsWith(sym, end_sym);
    }

    set<Symbol> startsWith (const Word& word) const 
    {
        set<Symbol> rez;

        for (const Symbol& sym : word) {
            make_union(rez, startsWith(sym));
            if (!isVanishing(sym)) {
                rez.erase(end_sym);
                break;
            }
        }
        
        return rez;
    }

    set<Symbol> startsWith (const Symbol& sym) const 
    {
        if (!PRODUKCIJE.count(sym)) 
            return ZAVRSNI.count(sym) ? set<Symbol>{sym} : set<Symbol>{end_sym};
        set<Symbol> rez;

        for (const Word& produkcija : PRODUKCIJE.at(sym))
            make_union(rez, startsWith(produkcija));
        
        return rez;
    }

    void dbgPrintFileLines () {
        for(auto l: fileLines_backup){
            cout << l << "\n";
        }
    }

    // //Ovom GDB debugger moze posluzit, iako je bol za set upat
    void printInfo()
    {
        cout << "Nezavrsni: \t";
        for(auto s: NEZAVRSNI){
            cout << s << " ";
        }

        cout << "\nZavrsni: \t";
        for(auto z: ZAVRSNI)
            cout << z << " ";

        cout << "\nSync Zavrsni: \t";
        for(auto z: SYNC_ZAVRSNI)
            cout << z << " ";

        cout << "\nProdukcije: \n";
        for(auto p: PRODUKCIJE){
            cout << "  " << p.first << " ::= ";
            for(auto pp: p.second){
                for(auto z: pp)
                    if (z == "$")
                        cout << "\"\" ";
                    else
                        cout << z << " ";
                if(pp != p.second.back())
                    cout << "| ";
            }
            cout << "\n";
        }
    }

    void dodajNoviPocetniZnak (const Symbol& newStartSym)
    {
        NEZAVRSNI.emplace(newStartSym);
        PRODUKCIJE[newStartSym] = {{BEGIN_SYMBOL}};
        BEGIN_SYMBOL = newStartSym;
    }
};

//PROVJERI:
struct LR1Item 
{
    Symbol left;               
    Word before_dot; 
    Word after_dot;
    set<Symbol> lookahead;

    LR1Item () : LR1Item("", {}) {}

    LR1Item(const Symbol& left) : LR1Item(left, {}) {}

    LR1Item (const Symbol& left, const Word& after_dot)
        : LR1Item(left, after_dot, {}) 
    {}

    LR1Item (const Symbol& left, const Word& after_dot, const set<Symbol>& lookahead)
        : LR1Item(left, {}, after_dot, lookahead) 
    {}

    LR1Item (const Symbol& left, const Word& before_dot, const Word& after_dot, const set<Symbol>& lookahead)
        : left(left), before_dot(before_dot), after_dot(reverse(after_dot)), lookahead(lookahead) 
    {}
    
    bool operator==(const LR1Item& other) const {
        return left == other.left &&
               before_dot == other.before_dot &&
               after_dot == other.after_dot &&
               lookahead == other.lookahead;
    }
    
    bool operator<(const LR1Item& other) const {
        if (left != other.left) return left < other.left;
        if (before_dot != other.before_dot) return before_dot < other.before_dot;
        if (after_dot != other.after_dot) return after_dot < other.after_dot;
        return lookahead < other.lookahead;
    }

    LR1Item &shift_dot_r () {
        before_dot.push_back(after_dot.back());
        after_dot.pop_back();
        return *this;
    }
    LR1Item &shift_dot_l () {
        after_dot.push_back(before_dot.back());
        before_dot.pop_back();
        return *this;
    }

    inline const Symbol& symbolAfterDot() {
        return after_dot.empty() ? end_sym : after_dot.back();
    }

    inline bool isComplete() {
        return symbolAfterDot() == end_sym;
    }

    //maknuo sam one cursed simbole sry, ak treba možeš vratit ali preferirao bi da izgleda ovako jer:
    //ako koristis stvari koje nisu u basic ASCII tablici postoji dobra sansa da se nece uvjek dobro ispisat
    // std::string toString() const 
    // { 
    //     std::string result = left + " -> ";
    //     for (const auto& s : before_dot) result += s + " ";
    //     result += ". ";
    //     for (const auto& s : after_dot) result += s + " ";
    //     result += ", " + lookahead;
    //     return result;
    // }
};

template <>
struct std::hash<LR1Item>
{
    std::size_t operator()(const LR1Item& k) const
    {
        return (
            (((std::hash<Symbol>()(k.left) ^ 
            (std::hash<Word>()(k.before_dot) << 1)) >> 1) ^ 
            (std::hash<Word>()(k.after_dot) << 1)) >> 1) ^ 
            (std::hash<set<Symbol>>()(k.lookahead) << 1);
    }
};