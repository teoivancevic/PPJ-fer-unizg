#pragma once

#include "utils.hpp"
#include <fstream>

class Grammar
{
    std::ifstream in;
    bool read_stdin = false;
    bool write_stdout = false;

public:
    Symbol BEGIN_SYMBOL;
    set<Symbol> NEZAVRSNI;
    set<Symbol> ZAVRSNI;
    set<Symbol> SYNC_ZAVRSNI;
    
    map<Symbol, vector<Word>> PRODUKCIJE;
    map<pair<Symbol, Word>, int> ID_PRODUKCIJE;
    int ID_global = 0;
    
    mutable map<pair<Symbol, Symbol>, bool> BEGINS_WITH; 
    mutable map<Symbol, set<Symbol>> BEGINS_WITH_ALL; 
    
    Grammar(const std::string& inputStream)
    {
        if(inputStream != "cin") 
            in = std::ifstream(inputStream);
        else 
            read_stdin = true;
        
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
                        cerr << "Error in file" <<endl;          
                }
                else if(line[0] == '<')
                {
                    currSymbol = line;
                }
                else if(line[0] == ' ')
                {
                    Word production = {};
                    Symbol symbolsString = line.substr(1, (int) line.size() - 1);

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
                    //usklađeno radi obrnutosti LR1Stavka.after_dot vectora
                    production = reverse(production); //REVERSE
                    
                    PRODUKCIJE[currSymbol].emplace_back(production);
                    ID_PRODUKCIJE[{currSymbol, production}] = ID_global++;
                }
                else
                    cerr << "Error in file" <<endl;
            }
            in.close();
        } 
        else
            cerr << "Unable to open file" <<endl;
    }

    Symbol readSymbol (const std::string line, set<Symbol>& container, int removeFirst = 3) 
    {
        Symbol symbolsString = line.substr(removeFirst, (int) line.size() - removeFirst);
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
        pair<Symbol, Symbol> key = {sym1, sym2};

        if (!exists(BEGINS_WITH, key)) 
        {
            BEGINS_WITH[key] = false;
            
            if (sym1 == sym2)
                BEGINS_WITH[key] = true;
            
            else if (PRODUKCIJE.count(sym1))
                for (const Word& produkcija : PRODUKCIJE.at(sym1))
                    if (startsWith(produkcija, sym2)) 
                        BEGINS_WITH[key] = true;
        }

        return BEGINS_WITH.at({sym1, sym2});
    }

    bool startsWith (const Word& word, const Symbol& sym2) const 
    {
        //usklađeno radi obrnutosti LR1Stavka.after_dot vectora
        for (int i = (int) word.size() - 1; i > -1; i--)  //REVERSE
        {
            const Symbol& sym1 = word[i];

            if (startsWith(sym1, sym2)) {
                if (sym2 != end_sym) return true;
            }
            else if (!isVanishing(sym1))
                return false;
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

        //isto usklađeno...
        for (int i = (int) word.size() - 1; i > -1; i--)  //REVERSE
        {
            const Symbol& sym = word[i];

            rez = make_union(rez, startsWith(sym));
            
            if (!isVanishing(sym)) break;
        }
        
        return rez;
    }

    set<Symbol> startsWith (const Symbol& sym) const 
    {
        if (!exists(BEGINS_WITH_ALL, sym)) 
        {
            BEGINS_WITH_ALL[sym] = {};

            if (!PRODUKCIJE.count(sym)) 
                BEGINS_WITH_ALL[sym] = ZAVRSNI.count(sym) ? set<Symbol>{sym} : set<Symbol>{};
            else {
                set<Symbol> rez;
                for (const Word& produkcija : PRODUKCIJE.at(sym))
                    rez = make_union(rez, startsWith(produkcija));
                
                BEGINS_WITH_ALL[sym] = rez;
            }
        }

        return BEGINS_WITH_ALL.at(sym);
    }

    inline bool isTerminating(const Symbol& sym) const {
        return (bool) ZAVRSNI.count(sym);
    }

    void dodajNoviPocetniZnak (const Symbol& newStartSym)
    {
        NEZAVRSNI.emplace(newStartSym);
        PRODUKCIJE[newStartSym] = {{BEGIN_SYMBOL}};
        BEGIN_SYMBOL = newStartSym;
    }
};

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
        : left(left), before_dot(before_dot), after_dot(after_dot), lookahead(lookahead) 
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