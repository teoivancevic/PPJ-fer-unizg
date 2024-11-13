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
    string content;

    Node(){
        symbol = "";
        state = -1;
        children = {};
        content = "";
    }

    Node(const string& sym) 
        : symbol(sym), state(-1), children({}), content("") {}
    Node(const string& sym, const string& cont) 
        : symbol(sym), state(-1), children({}), content(cont) {}
    Node(const int& st) 
        : symbol(""), state(st), children({}), content("") {}
    
    ~Node() {
        for (auto child : children) {
            delete child;
        }
    }

    bool isTerminal() const {
        return children.empty();
    }
};


void printStackDebug(const stack<Node*>& s) {
    // We need to make a copy since we can't iterate a stack directly
    stack<Node*> temp = s;
    vector<string> elements;
    
    // Store elements in reverse order (since stack prints from top)
    while (!temp.empty()) {
        Node* node = temp.top();
        if (node->state != -1) {
            elements.push_back(std::to_string(node->state));
        } else {
            elements.push_back(node->symbol);
        }
        temp.pop();
    }
    
    // Print header
    cerr << "\n=== Stack Bottom ===\n";
    
    // Print in correct order (bottom to top)
    for (int i = elements.size() - 1; i >= 0; i--) {
        cerr << elements[i] << " ";
    }
    cerr << "\n";
    
    cerr << "=== Stack Top ===\n\n";
}

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
    
    Node* cinAndPrint() {
        stack<Node*> nodeStack;
        nodeStack.emplace(new Node(0));

        string inputLineTemp;
        vector<string> inputLines;
        while(getline(std::cin, inputLineTemp)) {
            inputLines.push_back(inputLineTemp);
            cerr << "Line: " << inputLineTemp << '\n';
        }

        inputLines.push_back("$");
        Node* rootNode = nullptr;
        
        int i = 0;
        while (i < inputLines.size()) {
            printStackDebug(nodeStack);
            
            if (nodeStack.empty()) {
                cerr << "Error: Stack is empty\n";
                return nullptr;
            }

            string inputLine = inputLines[i];
            std::istringstream iss(inputLine);
            string symbol;
            iss >> symbol;
            
            Node* currentState = nodeStack.top();
            cerr << "Vrh stoga i ulazni simbol: " << currentState->state << " --- " << symbol << '\n';
            
            // Look up action in parsing table
            auto actionIt = table.akcija.find({currentState->state, symbol});
            if (actionIt == table.akcija.end()) {
                cerr << "Error recovery: No action defined for state " << currentState->state 
                    << " and symbol " << symbol << '\n';
                
                // Error recovery using sync sets
                if (table.SYNC_ZAVRSNI.find(symbol) != table.SYNC_ZAVRSNI.end()) {
                    cerr << "Found sync symbol '" << symbol << "', attempting recovery...\n";
                    
                    // Pop states until we find one that can handle this sync symbol
                    bool recovered = false;
                    while (!nodeStack.empty()) {
                        Node* state = nodeStack.top();
                        auto recoveryAction = table.akcija.find({state->state, symbol});
                        if (recoveryAction != table.akcija.end()) {
                            cerr << "Recovery successful at state " << state->state << '\n';
                            recovered = true;
                            break;
                        }
                        cerr << "Popping state " << state->state << " during recovery\n";
                        nodeStack.pop();
                    }
                    
                    if (!recovered) {
                        cerr << "Error recovery failed - could not find suitable state\n";
                        return nullptr;
                    }
                    continue;  // Try parsing again with the current symbol
                }
                
                // Skip erroneous input if not a sync symbol
                cerr << "Skipping invalid input: " << symbol << '\n';
                i++;
                continue;
            }

            string action = actionIt->second.first;
            int actionValue = actionIt->second.second;
            
            if (action == "POMAKNI") {
                cerr << "POMAKNI " << actionValue << '\n';
                
                // Create new nodes for symbol and state
                Node* symbolNode = new Node(symbol, inputLine);
                Node* newStateNode = new Node(actionValue);
                
                // Push in correct order
                nodeStack.emplace(symbolNode);
                nodeStack.emplace(newStateNode);
                i++;
                
            } else if (action == "REDUCIRAJ") {
                cerr << "REDUCIRAJ " << actionValue << '\n';
                
                // Get production rule
                auto production = table.ID_PRODUKCIJE_MAPA.at(actionValue);
                cerr << "Reducing using production " << actionValue << ": " 
                    << production.first << " -> ";
                for (const auto& sym : production.second) {
                    cerr << sym << " ";
                }
                cerr << '\n';
                
                // Create new node for the reduced non-terminal
                Node* newNode = new Node(production.first);
                
                // Pop nodes for each symbol in the production's right-hand side
                for (size_t j = 0; j < production.second.size(); j++) {
                    if (nodeStack.size() < 2) {
                        cerr << "Error: Stack underflow during reduction\n";
                        return nullptr;
                    }
                    
                    nodeStack.pop();  // Pop state
                    Node* child = nodeStack.top();  // Get symbol
                    nodeStack.pop();  // Pop symbol
                    
                    // Add as child in correct order
                    newNode->children.insert(newNode->children.begin(), child);
                }
                
                // Look up goto action
                if (nodeStack.empty()) {
                    cerr << "Error: Stack empty before goto\n";
                    return nullptr;
                }
                
                auto gotoIt = table.novoStanje.find({nodeStack.top()->state, newNode->symbol});
                if (gotoIt == table.novoStanje.end()) {
                    cerr << "Error: No goto action found for state " << nodeStack.top()->state 
                        << " and symbol " << newNode->symbol << '\n';
                    return nullptr;
                }
                
                cerr << "Goto: State " << nodeStack.top()->state 
                    << " with " << newNode->symbol 
                    << " -> " << gotoIt->second.second << '\n';
                
                if (gotoIt->second.first == "STAVI") {
                    nodeStack.emplace(newNode);
                    nodeStack.emplace(new Node(gotoIt->second.second));
                }
                
                rootNode = newNode;
                
            } else if (action == "PRIHVATI") {
                cerr << "PRIHVATI" << '\n';
                return rootNode;
            } else {
                cerr << "Error: Unknown action " << action << '\n';
                return nullptr;
            }
        }

        cerr << "Warning: Reached end of input without explicit accept\n";
        return rootNode;
    }

    void printFromRoot(Node* root, int depth = 0) {
        if (!root) return;  // Add this check to prevent segmentation fault
        
        // Print indentation and symbol
        if(root->isTerminal()){
            std::cout << string(depth, ' ') << root->content;
        }
        else{
            std::cout << string(depth, ' ') << root->symbol;
        }
        std::cout << '\n';
        
        // Recursively print all children
        for (auto child : root->children) {
            printFromRoot(child, depth + 1);
        }
    }
};




int main() {
    ParsingTableStuff table("tablica.txt");
    // table.printLoaded();
    std::cerr << "Parsing table constructed" << std::endl;

    SyntaxAnalyzer analyzer(table);
    cerr << "Analyzer constructed" << std::endl;
    Node* root = analyzer.cinAndPrint();

    analyzer.printFromRoot(root);

    
    return 0;
}