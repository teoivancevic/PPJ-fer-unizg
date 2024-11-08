#pragma once

#include "Utils.hpp"
#include <fstream>

//Dodo sam neka preimenovanja jer mi nisu bila konzistentna
//States bi trebala biti stanja automata, a Symbols znakovi gramatike, drzimo se toga

class Grammar
{
private:
    vector<std::string> fileLines_backup;
public:

    Symbol BEGIN_SYMBOL; // pocetni nezavrsni znak gramatike
    set<Symbol> NEZAVRSNI; // nezavrsni znakovi
    set<Symbol> ZAVRSNI; // zavrsni znakovi 
        //(note): mozda bolje drzat se engleskog TERMINATING
    set<Symbol> SYNC_ZAVRSNI; // sinkronizacijski zavrsni znakovi
    map<Symbol, vector<vector<Symbol>>> PRODUKCIJE;

    // parse input file
    Grammar(std::string filePath) 
    {
        std::ifstream file(filePath);
        std::string line;

        if (file.is_open())
        {
            Symbol currSymbol = "";
            while(getline(file, line)){
                //cout << line << "\n";
                if(line[0] == '%')
                {
                    if(line[1] == 'V')
                        BEGIN_SYMBOL = readSymbol(line, NEZAVRSNI);
                    else if(line[1] == 'T') 
                        readSymbol(line, ZAVRSNI); 
                    else if(line.substr(1, 3) == "Syn") 
                        readSymbol(line, SYNC_ZAVRSNI);
                    else 
                        cerr << "Error in file\n";          
                }
                else if(line[0] == '<')
                {
                    currSymbol = line;
                }
                else if(line[0] == ' ')
                {
                    vector<Symbol> production = {};
                    Symbol symbolsString = line.substr(1, line.size()-1);

                    Symbol symbol = "";
                    for(auto c: symbolsString){
                        if(c == ' '){
                            production.emplace_back(symbol);
                            symbol = "";
                        }else{
                            symbol += c;
                        }
                    }
                    production.emplace_back(symbol);

                    PRODUKCIJE[currSymbol].push_back(production);
                }
                else
                {
                    /* code */
                    cerr << "Error in file\n";
                }

                fileLines_backup.push_back(line);
            }
            file.close();
        } 
        else
            cerr << "Unable to open file\n";
    }

    //code duplication
    Symbol readSymbol (const std::string line, set<Symbol>& container) {
        Symbol symbolsString = line.substr(3, line.size()-3);

        Symbol symbol = "";
        for (auto c: symbolsString) {
            if (c == ' ') {
                container.emplace(symbol);
                symbol = "";
            } else {
                symbol += c;
            }
        }
        container.emplace(symbol);

        return symbol;
    }

    void dbgPrintFileLines () {
        for(auto l: fileLines_backup){
            cout << l << "\n";
        }
    }

    //Ovom GDB debugger moze posluzit, iako je bol za set upat
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

    void dodajNoviPocetniZnak ()
    {
        // dodajemo novi pocetni znak
        Symbol newStartSym = "<<q0>>";
        NEZAVRSNI.emplace(newStartSym);
        PRODUKCIJE[newStartSym] = {{BEGIN_SYMBOL}};
        BEGIN_SYMBOL = newStartSym;
    }
};