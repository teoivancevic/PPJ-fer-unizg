#include "FRISCGenerator.hpp"

using namespace TreeUtils;

void FRISCGenerator::IzrazProcessor::process_primarni_izraz(Node *node, bool emitCode) {
    if (node->children[0]->content.find("BROJ") == 0) {
        int value = stoi(node->children[0]->lexicalUnit);
        node->evaluatedValue = value;
        if (emitCode) {
            emit("    MOVE %D " + to_string(value) + ", R6");
        }
    }
    else if (node->children[0]->content.find("ZNAK") == 0) {
        string charLiteral = node->children[0]->lexicalUnit;
        int ascii = charLiteral[1];
        node->evaluatedValue = ascii;
        if (emitCode) {
            emit("    MOVE %D " + to_string(ascii) + ", R6");
        }
    }
    else if (node->children[0]->content.find("IDN") == 0) {
        string varName = node->children[0]->lexicalUnit;
        if (emitCode) {
            emit("    LOAD R6, (G_" + varName + ")");
        }
    }
    else if (node->children.size() == 3) {
        process_izraz(node->children[1], emitCode);
        node->evaluatedValue = node->children[1]->evaluatedValue;
    }
}

void FRISCGenerator::IzrazProcessor::process_postfiks_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_primarni_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
    }
}

void FRISCGenerator::IzrazProcessor::process_lista_argumenata(Node *node, bool emitCode) {
    // To be implemented if needed
}

void FRISCGenerator::IzrazProcessor::process_unarni_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_postfiks_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
    }
}

void FRISCGenerator::IzrazProcessor::process_unarni_operator(Node *node, bool emitCode) {
    // To be implemented if needed
}

void FRISCGenerator::IzrazProcessor::process_cast_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_unarni_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
    }
}

void FRISCGenerator::IzrazProcessor::process_multiplikativni_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_cast_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
    }
}

void FRISCGenerator::IzrazProcessor::process_aditivni_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_multiplikativni_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
    }
    else {
        // Process left and right operands
        process_aditivni_izraz(node->children[0], emitCode);
        int leftValue = node->children[0]->evaluatedValue;

        if (emitCode) {
            emit("    PUSH R6");  // Save left value
        }
        
        process_multiplikativni_izraz(node->children[2], emitCode);
        int rightValue = node->children[2]->evaluatedValue;
        
        if (emitCode) {
            emit("    POP R0");   // Get left value into R0
            
            if (node->children[1]->content.find("PLUS") == 0) {
                emit("    ADD R0, R6, R6");
                node->evaluatedValue = leftValue + rightValue;
            }
            else if (node->children[1]->content.find("MINUS") == 0) {
                emit("    SUB R0, R6, R6");
                node->evaluatedValue = leftValue - rightValue;
            }
        } else {
            if (node->children[1]->content.find("PLUS") == 0) {
                node->evaluatedValue = leftValue + rightValue;
            }
            else if (node->children[1]->content.find("MINUS") == 0) {
                node->evaluatedValue = leftValue - rightValue;
            }
        }
    }
}

void FRISCGenerator::IzrazProcessor::process_odnosni_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_aditivni_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
    }
}

void FRISCGenerator::IzrazProcessor::process_jednakosni_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_odnosni_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
    }
}

void FRISCGenerator::IzrazProcessor::process_bin_i_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_jednakosni_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
    }
}

void FRISCGenerator::IzrazProcessor::process_bin_xili_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_bin_i_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
    }
}

void FRISCGenerator::IzrazProcessor::process_bin_ili_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_bin_xili_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
    }
}

void FRISCGenerator::IzrazProcessor::process_log_i_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_bin_ili_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
    }
}

void FRISCGenerator::IzrazProcessor::process_log_ili_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_log_i_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
    }
}

void FRISCGenerator::IzrazProcessor::process_izraz_pridruzivanja(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_log_ili_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
    }
}

void FRISCGenerator::IzrazProcessor::process_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_izraz_pridruzivanja(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
    }
    else if (node->children.size() == 3) {
        process_izraz(node->children[0], emitCode);
        process_izraz_pridruzivanja(node->children[2], emitCode);
        node->evaluatedValue = node->children[2]->evaluatedValue;
    }
}