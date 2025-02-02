#include "FRISCGenerator.hpp"

using namespace TreeUtils;

void FRISCGenerator::IzrazProcessor::process_primarni_izraz(Node *node, bool emitCode) {
    if (node->children[0]->content.find("BROJ") == 0) {
        // Use long long for intermediate calculation to avoid overflow
        long long value = stoll(node->children[0]->lexicalUnit);
        // Store as 32-bit int for FRISC
        node->evaluatedValue = static_cast<int>(value);
        if (emitCode) {
            emit("    MOVE %D " + to_string(node->evaluatedValue) + ", R6");
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
        node->evaluatedValueString = varName;
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
        node->evaluatedValueString = node->children[0]->evaluatedValueString;
        node->lineNumber = node->children[0]->lineNumber;
    }
    else if (node->children[1]->content.find("L_UGL_ZAGRADA") == 0) {
        // Array access
        string arrayName = node->children[0]->children[0]->lexicalUnit;
        process_izraz(node->children[2], emitCode);  // Get index into R6
        
        if (emitCode) {
            // Calculate array offset
            emit("    MOVE %D 4, R0");         // Each element is 4 bytes
            emit("    MUL R6, R0, R0");        // Multiply index by 4
            emit("    MOVE %D G_" + arrayName + ", R1");  // Get array base address
            emit("    ADD R0, R1, R1");        // Add offset to base
            emit("    LOAD R6, (R1)");         // Load value from calculated address
        }
    }
    else if (node->children.size() == 3 && 
             node->children[1]->content.find("L_ZAGRADA") == 0) {
        // Function call without arguments
        // Call process_primarni_izraz to handle the function name
        process_primarni_izraz(node->children[0], false);  // Don't emit code yet
        
        if (emitCode) {
            string funcName = node->children[0]->evaluatedValueString;
            cerr << "FUNC NAME FROM POSFTIKS " << funcName << "."<< endl;
            emit("    CALL F_" + funcName);
        }
        // The return value should already be in R6
    }
}

void FRISCGenerator::IzrazProcessor::process_lista_argumenata(Node *node, bool emitCode) {
    // To be implemented if needed
}


void FRISCGenerator::IzrazProcessor::process_unarni_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        // Just a postfiks_izraz
        process_postfiks_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
        node->lineNumber = node->children[0]->lineNumber;
    }
    else if (node->children.size() == 2) {
        // Unary operator case
        if (node->children[0]->symbol == "<unarni_operator>") {
            // Get the operator
            string op = node->children[0]->children[0]->content;
            
            // Process the operand
            process_cast_izraz(node->children[1], emitCode);
            int value = node->children[1]->evaluatedValue;
            
            if (op.find("MINUS") == 0) {
                // Handle unary minus
                value = -value;
                if (emitCode) {
                    // Load operand into R6 (already done by cast_izraz)
                    // Then negate it
                    emit("    MOVE %D 0, R0");
                    emit("    SUB R0, R6, R6");
                }
            }
            node->evaluatedValue = value;
        }
    }
}

void FRISCGenerator::IzrazProcessor::process_unarni_operator(Node *node, bool emitCode) {
    // To be implemented if needed
}

void FRISCGenerator::IzrazProcessor::process_cast_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_unarni_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
        node->lineNumber = node->children[0]->lineNumber;
    }
}

void FRISCGenerator::IzrazProcessor::process_multiplikativni_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_cast_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
        node->lineNumber = node->children[0]->lineNumber;
    }
}

void FRISCGenerator::IzrazProcessor::process_aditivni_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_multiplikativni_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
        node->lineNumber = node->children[0]->lineNumber;
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
    else {
        // Process left operand
        process_odnosni_izraz(node->children[0], emitCode);
        if (emitCode) {
            emit("    PUSH R6");  // Save left value
        }
        
        // Process right operand
        process_aditivni_izraz(node->children[2], emitCode);
        if (emitCode) {
            emit("    POP R0");   // Get left value into R0
            
            // Compare R0 (left) with R6 (right)
            emit("    CMP R0, R6");
            
            string op = node->children[1]->content;
            if (op.find("OP_LT") != string::npos) {
                emit("    JP_SLT SKIP_" + to_string(FG->labelCounter));
            }
            else if (op.find("OP_GT") != string::npos) {
                emit("    JP_SGT SKIP_" + to_string(FG->labelCounter));
            }
            else if (op.find("OP_LTE") != string::npos) {
                emit("    JP_SLE SKIP_" + to_string(FG->labelCounter));
            }
            else if (op.find("OP_GTE") != string::npos) {
                emit("    JP_SGE SKIP_" + to_string(FG->labelCounter));
            }
            
            // If comparison failed, set R6 to 0
            emit("    MOVE %D 0, R6");
            emit("    JP CONTINUE_" + to_string(FG->labelCounter));
            
            // If comparison succeeded, set R6 to 1
            emit("SKIP_" + to_string(FG->labelCounter));
            emit("    MOVE %D 1, R6");
            emit("CONTINUE_" + to_string(FG->labelCounter++));
        }
    }
}

void FRISCGenerator::IzrazProcessor::process_jednakosni_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_odnosni_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
        node->lineNumber = node->children[0]->lineNumber;
    }
}

void FRISCGenerator::IzrazProcessor::process_bin_i_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_jednakosni_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
        node->lineNumber = node->children[0]->lineNumber;
    }
    else {
        // Process left and right operands
        process_bin_i_izraz(node->children[0], emitCode);
        int leftValue = node->children[0]->evaluatedValue;

        if (emitCode) {
            emit("    PUSH R6");  // Save left value
        }
        
        process_jednakosni_izraz(node->children[2], emitCode);
        int rightValue = node->children[2]->evaluatedValue;
        
        if (emitCode) {
            emit("    POP R0");   // Get left value into R0
            
            // Bitwise AND operation
            emit("    AND R0, R6, R6");
            node->evaluatedValue = leftValue & rightValue;
        } else {
            node->evaluatedValue = leftValue & rightValue;
        }
    }
}

void FRISCGenerator::IzrazProcessor::process_bin_xili_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_bin_i_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
        node->lineNumber = node->children[0]->lineNumber;
    }
    else {
        // Process left and right operands
        process_bin_xili_izraz(node->children[0], emitCode);
        int leftValue = node->children[0]->evaluatedValue;

        if (emitCode) {
            emit("    PUSH R6");  // Save left value
        }
        
        process_bin_i_izraz(node->children[2], emitCode);
        int rightValue = node->children[2]->evaluatedValue;
        
        if (emitCode) {
            emit("    POP R0");   // Get left value into R0
            
            // Bitwise XOR operation
            emit("    XOR R0, R6, R6");
            node->evaluatedValue = leftValue ^ rightValue;
        } else {
            node->evaluatedValue = leftValue ^ rightValue;
        }
    }
}

void FRISCGenerator::IzrazProcessor::process_bin_ili_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_bin_xili_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
        node->lineNumber = node->children[0]->lineNumber;
    }
    else {
        // Process left and right operands
        process_bin_ili_izraz(node->children[0], emitCode);
        int leftValue = node->children[0]->evaluatedValue;

        if (emitCode) {
            emit("    PUSH R6");  // Save left value
        }
        
        process_bin_xili_izraz(node->children[2], emitCode);
        int rightValue = node->children[2]->evaluatedValue;
        
        if (emitCode) {
            emit("    POP R0");   // Get left value into R0
            
            // Bitwise OR operation
            emit("    OR R0, R6, R6");
            node->evaluatedValue = leftValue | rightValue;
        } else {
            node->evaluatedValue = leftValue | rightValue;
        }
    }
}

void FRISCGenerator::IzrazProcessor::process_log_i_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_bin_ili_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
        node->lineNumber = node->children[0]->lineNumber;
    }
}

void FRISCGenerator::IzrazProcessor::process_log_ili_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_log_i_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
        node->lineNumber = node->children[0]->lineNumber;
    }
}

void FRISCGenerator::IzrazProcessor::process_izraz_pridruzivanja(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        process_log_ili_izraz(node->children[0], emitCode);
        node->evaluatedValue = node->children[0]->evaluatedValue;
        node->lineNumber = node->children[0]->lineNumber;
    }
}

void FRISCGenerator::IzrazProcessor::process_izraz(Node *node, bool emitCode) {
    if (node->children.size() == 1) {
        // Single expression
        Node* exprNode = node->children[0];
        process_izraz_pridruzivanja(exprNode, emitCode);
        
        // Preserve the line number
        node->lineNumber = exprNode->lineNumber;
        node->evaluatedValue = exprNode->evaluatedValue;
    }
    else if (node->children.size() == 3) {
        // Multiple expressions separated by comma
        process_izraz(node->children[0], emitCode);
        process_izraz_pridruzivanja(node->children[2], emitCode);
        
        // Use the line number of the last expression
        node->lineNumber = node->children[2]->lineNumber;
        node->evaluatedValue = node->children[2]->evaluatedValue;
    }
}