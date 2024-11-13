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


class GenTreeNode {
public:
    string symbol;              
    vector<GenTreeNode*> children;
    int line;               
    string value;              
    bool isTerminal;       

    // Constructor for terminals
    GenTreeNode(const string& sym, const int& lineNum, const string& val) 
        : symbol(sym), line(lineNum), value(val), isTerminal(true) {}
    
    // Constructor for non-terminals
    GenTreeNode(const string& sym) 
        : symbol(sym), line(0), isTerminal(false) {}
    
    ~GenTreeNode() {
        for (auto child : children) {
            delete child;
        }
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
public:
    SyntaxAnalyzer(const ParsingTableStuff& table) : table(table) {}

    GenTreeNode* parse() {
        vector<pair<string, pair<int, string>>> tokens = readTokensFromCin();
        stack<int> stateStack;
        stack<GenTreeNode*> nodeStack;
        
        stateStack.push(0);  // Initial state
        int currentToken = 0;
        cerr << "Tokens size " << tokens.size() << '\n';
        cerr << "Parsing started" << '\n';


        while (currentToken <= tokens.size()) {
            int currentState = stateStack.top();
            string currentSymbol;
            
            if (currentToken == tokens.size()) {
                currentSymbol = "$";
            } else {
                currentSymbol = tokens[currentToken].first;
            }

            // Get action
            auto actionIt = table.akcija.find({currentState, currentSymbol});
            if (actionIt == table.akcija.end()) {
                // Error handling
                if (table.SYNC_ZAVRSNI.count(currentSymbol)) {
                    // Skip until sync symbol
                    currentToken++;
                    continue;
                }
                // std::cerr << "No action defined for state " + to_string(currentState) + " and symbol " + currentSymbol;
            }

            const string& action = actionIt->second.first;
            if (action.substr(0, 7) == "POMAKNI") {
                // Shift operation
                int nextState = actionIt->second.second;  // Use the second part of the pair
                
                // Create new node for terminal
                auto token = tokens[currentToken];
                GenTreeNode* node = new GenTreeNode(token.first, token.second.first, token.second.second);
                
                nodeStack.push(node);
                stateStack.push(nextState);
                currentToken++;
            }
            else if (action.substr(0, 8) == "REDUCIRAJ") {
                // Reduce operation
                int prodId = actionIt->second.second;  // Use the second part of the pair
                auto prod = table.ID_PRODUKCIJE_MAPA.at(prodId);
                
                // Create new node for non-terminal
                GenTreeNode* newNode = new GenTreeNode(prod.first);
                
                // Pop states and nodes according to production length
                for (int i = 0; i < prod.second.size(); i++) {
                    stateStack.pop();
                    if (!nodeStack.empty()) {
                        GenTreeNode* child = nodeStack.top();
                        nodeStack.pop();
                        newNode->children.insert(newNode->children.begin(), child);
                    }
                }
                
                // Get new state
                currentState = stateStack.top();
                auto gotoIt = table.novoStanje.find({currentState, prod.first});
                if (gotoIt == table.novoStanje.end()) {
                    std::cerr << "No goto defined for state " << currentState << " and non-terminal " + prod.first;
                }
                
                //string gotoAction = gotoIt->second.first;
                int nextState = gotoIt->second.second;  // Use the second part of the pair
                
                nodeStack.push(newNode);
                stateStack.push(nextState);
            }
            else if (action.substr(0, 8) == "PRIHVATI") {
                if (!nodeStack.empty()) {
                    return nodeStack.top();
                }
                return nullptr;
            }
            
            currentToken++;
            cerr << "Current token: " << currentToken << '\n';
        }
        
        return nullptr;
    }

private:
    const ParsingTableStuff& table;

    vector<pair<string, pair<int, string>>> readTokensFromCin() {
        vector<pair<string, pair<int, string>>> tokens;
        string terminal, line, value;
        
        while (std::cin >> terminal >> line >> value) {
            tokens.push_back({terminal, {stoi(line), value}});
        }
        
        return tokens;
    }
};

// Print tree function
void printTree(GenTreeNode* node, int depth = 0) {
    if (!node) return;
    
    string indent(depth, ' ');
    std::cout << indent << node->symbol;
    if (node->isTerminal) {
        std::cout << " " << node->line << " " << node->value;
    }
    // cout << endl;
    printf("\n");
    
    for (auto child : node->children) {
        printTree(child, depth + 1);
    }
}

int main() {
    ParsingTableStuff table("tablica.txt");
    std::cerr << "Parsing table constructed" << std::endl;
    SyntaxAnalyzer analyzer(table);
    std::cerr << "Syntax analyzer constructed" << std::endl;
    
    GenTreeNode* tree = analyzer.parse();
    std::cerr << "Parsing done" << '\n';
    
    printTree(tree);
    if (tree) {
        printTree(tree);
        delete tree;
    } else {
        std::cout << "Parsing failed" << '\n';
    }
    
    return 0;
}