#include <map>
#include <set>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stack>


using std::cerr;

using std::map;
using std::set;
using std::vector;
using std::pair;
using std::string;
using std::stack;

using Symbol = std::string;
using State = int;
using Action = std::pair<string, int>;

using Word = vector<Symbol>;

struct Node {
public:
    string symbol;
    int state;
    vector<Node*> children;

    Node(){
        symbol = "";
        state = -1;
        children = {};
    }

    Node(const string& sym) 
        : symbol(sym), state(-1), children({}) {}
    Node(const int& st) 
        : symbol(""), state(st), children({}) {}
    
    ~Node() {
        for (auto child : children) {
            delete child;
        }
    }

    bool isTerminal() const {
        return children.empty();
    }
};

class ParsingTableStuff {
public:
    map<pair<int, Symbol>, Action> akcija;      
    map<pair<int, Symbol>, Action> novoStanje;  
    set<Symbol> SYNC_ZAVRSNI;
    map<int, pair<Symbol, Word>> ID_PRODUKCIJE_MAPA;

    ParsingTableStuff(const string& filename) {
        std::ifstream in(filename);
        string line;
        
        // Read SYNC_SYMBOLS
        getline(in, line); // Read "SYNC_SYMBOLS:"
        while (getline(in, line) && !line.empty()) {
            SYNC_ZAVRSNI.insert(line);
        }

        // Read GRAMMAR_PRODUCTIONS:
        getline(in, line); // "GRAMMAR_PRODUCTIONS:"
        while (getline(in, line) && !line.empty()) {
            int id;
            Symbol left;
            Word right;
            
            // Find positions of key elements
            size_t space_pos = line.find(' ');
            id = stoi(line.substr(0, space_pos));
            
            size_t arrow_pos = line.find("->");
            left = line.substr(space_pos + 1, arrow_pos - space_pos - 2);
            
            // Get right side of production
            string right_side = line.substr(arrow_pos + 3);
            
            // Split right side into symbols
            size_t pos = 0;
            string token;
            while (pos < right_side.length()) {
                size_t next_pos = right_side.find(' ', pos);
                if (next_pos == string::npos) {
                    token = right_side.substr(pos);
                    if (!token.empty()) right.push_back(token);
                    break;
                }
                token = right_side.substr(pos, next_pos - pos);
                if (!token.empty()) right.push_back(token);
                pos = next_pos + 1;
            }
            
            ID_PRODUKCIJE_MAPA[id] = {left, right};
        }
        // std::cerr << "Grammar productions loaded" << std::endl;

        // Read AKCIJA:
        getline(in, line); // "AKCIJA:"
        while (getline(in, line) && !line.empty()) {
            int state;
            Symbol symbol;
            Action action;
            
            // Find positions of spaces
            size_t first_space = line.find(' ');
            size_t second_space = line.find(' ', first_space + 1);
            
            state = stoi(line.substr(0, first_space));
            symbol = line.substr(first_space + 1, second_space - first_space - 1);
            string actionString = line.substr(second_space + 1);
            first_space = actionString.find(' ');
            if (first_space == std::string::npos) {
                action = {actionString, -1}; // ako je PRIHVATI stanje
            } else {
                action = {actionString.substr(0, first_space), stoi(actionString.substr(first_space + 1))};
            }
            
            akcija[{state, symbol}] = action;
        }

        // std::cerr << "AKCIJA loaded" << std::endl;

        // Read NOVO STANJE:
        getline(in, line); // "NOVO STANJE:"
        while (getline(in, line) && !line.empty()) {
            int state;
            Symbol symbol;
            Action action;
            
            // Find positions of spaces
            size_t first_space = line.find(' ');
            size_t second_space = line.find(' ', first_space + 1);
            
            state = stoi(line.substr(0, first_space));
            symbol = line.substr(first_space + 1, second_space - first_space - 1);
            
            string actionString = line.substr(second_space + 1);
            first_space = actionString.find(' ');
            action = {actionString.substr(0, first_space), stoi(actionString.substr(first_space + 1))};
            
            novoStanje[{state, symbol}] = action;
        }

        // std::cerr << "NOVO STANJE loaded" << std::endl;
    }

    // Debug function to verify loaded data
    void printLoaded() const {
        std::cout << "SYNC_SYMBOLS:\n";
        for (const auto& sym : SYNC_ZAVRSNI) {
            std::cout << sym << "\n";
        }
        std::cout << "\nGRAMMAR_PRODUCTIONS:\n";
        for (const auto& [id, prod] : ID_PRODUKCIJE_MAPA) {
            std::cout << id << ", " << prod.first << " :== ";
            for (const auto& s : prod.second) {
                std::cout << s << ", ";
            }
            std::cout << "\n";
        }
        std::cout << "\nAKCIJA:\n";
        for (const auto& [key, value] : akcija) {
            std::cout << key.first << ", " << key.second << ", " << value.first << ", " << value.second << "\n";
        }
        std::cout << "\nNOVO STANJE:\n";
        for (const auto& [key, value] : novoStanje) {
            std::cout << key.first << ", " << key.second << ", " << value.first << ", " << value.second << "\n";
        }
    }
};

class SyntaxAnalyzer {
private:
    const ParsingTableStuff& table;
    
public:
    
    SyntaxAnalyzer(const ParsingTableStuff& table) : table(table) {}
    
    void cinAndPrint(){
        stack<Node*> nodeStack;
        nodeStack.emplace(new Node(0));

        // getline from cin
        string inputLine;
        while (getline(std::cin, inputLine)) {
            std::istringstream iss(inputLine);
            string symbol;
            iss >> symbol;
            
            Node* node = nodeStack.top();
            auto actionIt = table.akcija.find({node->state, symbol});
            if (actionIt == table.akcija.end()) {
                // NEMA DEFINIRANE AKCIJE
                // oporavak od pogreske?
                cerr << "NEMA DEFINIRANE AKCIJE // oporavak od pogreske?" << '\n';
            }

            string action = actionIt->second.first; // ovo je string koji opisuje akciju
            int id_next = actionIt->second.second; // ovo je id stanja na koje treba preci
            if (action == "POMAKNI") {
                cerr << "POMAKNI " << id_next << '\n';
                nodeStack.emplace(new Node(inputLine));
                nodeStack.emplace(new Node(id_next));
            } else if (action == "REDUCIRAJ") {
                cerr << "REDUCIRAJ " << id_next << '\n';
                auto production = table.ID_PRODUKCIJE_MAPA.at(id_next);
                Node *newNode = new Node(production.first); // novi "parent" node
                
                for (int i = 0; i < production.second.size(); i++) { // broj elemenata u desnoj strani produkcije
                    nodeStack.pop(); // makni stanje
                    newNode->children.insert(newNode->children.begin(), nodeStack.top()); // makni symbol i stavi ga u novi node kao child
                }
                // nodeStack.emplace(newNode); 
                auto newStateIt = table.novoStanje.find({nodeStack.top()->state, production.first}); // novo stanje
                if (newStateIt == table.novoStanje.end()) {
                    cerr << "Odbaci usred redukcije????" << '\n';
                }
                else if(newStateIt->second.first == "STAVI"){
                    nodeStack.emplace(newNode); // stavi novi node na stog
                }
            } else if (action == "PRIHVATI") {
                cerr << "PRIHVATI" << '\n';
                return;
            } else {
                cerr << "Error: Unknown action " << action << '\n';
                return;
            }
            
        }

    }

    void printFromRoot(Node* root, int depth = 0) {
        if (!root) return;
        std::cout << string(depth, ' ') << root->symbol << '\n';
        for (auto child : root->children) {
            printFromRoot(child, depth + 1);
        }
    }
};


int main() {
    ParsingTableStuff table("tablica.txt");
    table.printLoaded();

    std::cerr << "Parsing table constructed" << std::endl;

    SyntaxAnalyzer analyzer(table);
    analyzer.cinAndPrint();

    
    return 0;
}