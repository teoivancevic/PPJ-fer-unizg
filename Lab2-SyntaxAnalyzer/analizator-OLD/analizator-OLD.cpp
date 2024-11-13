#include <map>
#include <set>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stack>


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
// First, let's define the tree node structure
class GenTreeNode {
public:
    Symbol symbol;              // Terminal or non-terminal symbol
    vector<GenTreeNode*> children;
    string line;               // For terminals: line number
    string value;              // For terminals: actual value

    GenTreeNode(const Symbol& sym) : symbol(sym) {}
    
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

    GenTreeNode* parse(const string& input_filename) {
        vector<pair<string, pair<int, string>>> tokens = readTokens(input_filename);
        
        stack<int> stateStack;
        stack<GenTreeNode*> symbolStack;
        
        stateStack.push(0);  // Initial state
        int currentToken = 0;
        
        while (currentToken <= tokens.size()) {
            int currentState = stateStack.top();
            Symbol currentSymbol = (currentToken == tokens.size()) ? "$" : 
                                 tokens[currentToken].first;
            
            auto actionIt = table.akcija.find({currentState, currentSymbol});
            if (actionIt == table.akcija.end()) {
                handleError(currentState, currentSymbol, tokens[currentToken].second.first);
                currentToken++;
                continue;
            }
            
            string action = actionIt->second;
            if (action.substr(0, 7) == "POMAKNI") {
                int nextState = stoi(action.substr(8));
                GenTreeNode* node = new GenTreeNode(currentSymbol);
                node->line = to_string(tokens[currentToken].second.first);
                node->value = tokens[currentToken].second.second;
                
                symbolStack.push(node);
                stateStack.push(nextState);
                currentToken++;
            }
            else if (action.substr(0, 8) == "REDUCIRAJ") {
                int prodId = stoi(action.substr(9));
                auto [left, right] = table.ID_PRODUKCIJE_MAPA.at(prodId);
                
                GenTreeNode* newNode = new GenTreeNode(left);
                
                // Pop right-hand side symbols and states
                for (int i = 0; i < right.size(); i++) {
                    stateStack.pop();
                    GenTreeNode* child = symbolStack.top();
                    symbolStack.pop();
                    newNode->children.insert(newNode->children.begin(), child);
                }
                
                // Get new state from NOVO_STANJE table
                int currentState = stateStack.top();
                auto novoStanjeIt = table.novoStanje.find({currentState, left});
                if (novoStanjeIt != table.novoStanje.end()) {
                    string newStateAction = novoStanjeIt->second;
                    int newState = stoi(newStateAction.substr(6));
                    symbolStack.push(newNode);
                    stateStack.push(newState);
                }
            }
            else if (action == "PRIHVATI") {
                return symbolStack.top();
            }
        }
        
        return nullptr;
    }

private:
    const ParsingTableStuff& table;

    vector<pair<string, pair<int, string>>> readTokens(const string& filename) {
        vector<pair<string, pair<int, string>>> tokens;
        ifstream in(filename);
        string token, line, value;
        
        while (in >> token >> line >> value) {
            tokens.push_back({token, {stoi(line), value}});
        }
        return tokens;
    }

    void handleError(int state, const Symbol& symbol, int line) {
        cerr << "Error at line " << line << ": Unexpected symbol " << symbol << endl;
        // Optional: Implement error recovery using SYNC_ZAVRSNI
    }
};

void printTree(GenTreeNode* node, int depth = 0) {
    if (!node) return;
    
    string indent(depth * 2, ' ');
    cout << indent << node->symbol;
    if (!node->value.empty()) {
        cout << "(" << node->line << "," << node->value << ")";
    }
    cout << endl;
    
    for (auto child : node->children) {
        printTree(child, depth + 1);
    }
}

// Test function
int main() {
    ParsingTableStuff table("tablica.txt");
    table.printLoaded(); // Print to verify correct loading

    GenTreeNode* tree = analyzer.parse("input.txt");
    if (tree) {
        printTree(tree);
        delete tree;
    } else {
        std::cerr << "Parsing failed" << '\n';
    }

    return 0;
}