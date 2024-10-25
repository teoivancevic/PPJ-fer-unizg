#include<iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>


const std::string file_path = "../test/lab2_teza/00aab_1/test.san";

using std::cin;
using std::cout;

using State = std::string;
using Symbol = std::string;


class Grammar{
private:
    std::vector<std::string> fileLines_backup;
public:
    // parse input file

    std::vector<State> STATES; // nezavrsni znakovi
    std::vector<Symbol> ZAVRSNI; // zavrsni znakovi
    std::vector<Symbol> SYNC_ZAVRSNI; // sinkronizacijski zavrsni znakovi
    std::map<State, std::vector<std::string>> PRODUKCIJE;

    Grammar(std::string filePath){
        std::ifstream file(file_path);
        std::string line;

        if(file.is_open()){
            State currState = "";
            while(getline(file, line)){
                //cout << line << "\n";
                // make switch case if the first characters are "%V" 
                if(line[0] == '%'){
                    if(line[1] == 'V')
                    {
                        // nezavrsni znakovi
                        std::string states = line.substr(3, line.size()-3);
                        STATES.push_back(states);
                    }
                    else if(line[1] == 'T')
                    {
                        // zavrsni znakovi
                        std::string symbols = line.substr(3, line.size()-3);
                        ZAVRSNI.push_back(symbols);
                    }
                    else if(line.substr(1, 3) == "Syn"){
                        // sinkronizacijski zavrsni znakovi
                        std::string sync_symbols = line.substr(5, line.size()-5);
                        SYNC_ZAVRSNI.push_back(sync_symbols);
                    }
                    else
                    {
                        std::cerr << "Error in file\n";   
                    }
                        
                }
                else if(line[0] == '<')
                {
                    currState = line;
                }
                else if(line[0] == ' ')
                {
                    PRODUKCIJE[currState].push_back(line);
                }
                else
                {
                    /* code */
                    std::cerr << "Error in file\n";
                }
                


                fileLines_backup.push_back(line);
            }
            file.close();
        }else{
            std::cerr << "Unable to open file\n";
        }
    }

    void dbgPrintFileLines(){
        for(auto l: fileLines_backup){
            cout << l << "\n";
        }
    }

    void printInfo(){

        cout << "Nezavrsni: \t";
        for(auto s: STATES){
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
                if(pp == p.second.back())
                    cout << pp;
                else
                    cout << pp << " | ";
            }
            cout << "\n";
        }
    }
};


// TODO: seperate all strings in input, and only then append them to the vector
//        the map of transitions should be key string, value vector of vector of strings 
//          (first vector is list of transition, second vector is a single transition, list of states and symbols)

int main(){
    
    Grammar grammar(file_path);
    grammar.printInfo();    

    return 0;
}