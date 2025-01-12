#include "SemanticAnalyzer.hpp"

using namespace TreeUtils;

void SemanticAnalyzer::IzrazProcessor::process_primarni_izraz(Node *node)
{
    cerr << "DEBUG: Entering process_primarni_izraz\n";

    // if (!node || (node->children.size() != 1 && node->children.size() != 3))
    // {
    //     cerr << "DEBUG: Invalid node structure in process_primarni_izraz\n";
    //     reportError(node);
    //     return;
    // }

    // Handle parenthesized expression
    if (node->children.size() == 3)
    {
        if (node->children[0]->content.find("L_ZAGRADA") == 0 &&
            node->children[2]->content.find("D_ZAGRADA") == 0)
        {
            cerr << "DEBUG: Parenthesized expression detected\n";
            process_izraz(node->children[1]);
            node->typeInfo = node->children[1]->typeInfo;
            node->isLValue = node->children[1]->isLValue;
            return;
        }
        cerr << "DEBUG: Invalid parenthesized expression\n";
        reportError(node);
        return;
    }

    Node *child = node->children[0];
    string childContent = child->content;

    // Handle identifier - validate existence immediately
    if (childContent.find("IDN") == 0)
    {
        cerr << "DEBUG: Processing identifier: " << child->lexicalUnit << endl;
        SymbolTableEntry *entry = currentScope->lookup(child->lexicalUnit);

        if (!entry)
        {
            cerr << "DEBUG: Identifier not found in current scope\n";
            reportError(node);
            return;
        }

        // if (entry->type.isFunc())
        // {
        //     cerr << "DEBUG: Identifier is a function, not allowed here\n";
        //     reportError(node);
        //     return;
        // }

        // Set type and l-value status for identifier
        node->typeInfo = entry->type;
        node->isLValue = entry->type.lValue();
        cerr << "DEBUG: Identifier type: " << node->typeInfo.toString() << ", l-value: " << node->isLValue << endl;
        return;
    }

    // Handle constants
    if (childContent.find("BROJ") == 0)
    {
        cerr << "DEBUG: Processing numeric constant: " << child->lexicalUnit << endl;
        if (!Validator::validateNumConstant(child->lexicalUnit, node->typeInfo))
        {
            cerr << "DEBUG: Invalid numeric constant\n";
            reportError(node);
            return;
        }
        node->isLValue = false;
        return;
    }
    else if (childContent.find("ZNAK") == 0)
    {
        cerr << "DEBUG: Processing character constant: " << child->lexicalUnit << endl;
        if (!Validator::validateCharConstant(child->lexicalUnit, node->typeInfo))
        {
            cerr << "DEBUG: Invalid character constant\n";
            reportError(node);
            return;
        }
        node->isLValue = false;
        return;
    }
    else if (childContent.find("NIZ_ZNAKOVA") == 0)
    {
        cerr << "DEBUG: Processing string constant: " << child->lexicalUnit << endl;
        if (!Validator::validateNizZnakova(child->lexicalUnit, node->typeInfo))
        {
            cerr << "DEBUG: Invalid string constant\n";
            reportError(node);
            return;
        }
        node->isLValue = false;
        return;
    }

    // cerr << "DEBUG: Unknown primary expression\n";
    // reportError(node);
}


void SemanticAnalyzer::IzrazProcessor::process_postfiks_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <postfiks_izraz> ::= <primarni_izraz>
    if (node->children.size() == 1)
    {
        process_primarni_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // Other postfix expressions have more children
    if (node->children.size() < 2)
    {
        reportError(node);
        return;
    }

    // Get first child's type and values
    process_postfiks_izraz(node->children[0]);

    // <postfiks_izraz> ::= <postfiks_izraz> L_UGL_ZAGRADA <izraz> D_UGL_ZAGRADA
    if (node->children[1]->content.find("L_UGL_ZAGRADA") == 0)
    {
        // Check array access
        if (!TypeUtils::isArrayType(node->children[0]->typeInfo))
        {
            reportError(node);
            return;
        }

        process_izraz(node->children[2]);
        if (!node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        // Result type is the array's element type
        BasicType baseType = node->children[0]->typeInfo.getBaseType();
        bool isConst = node->children[0]->typeInfo.isConst();
        node->typeInfo = TypeInfo(baseType, isConst);
        node->isLValue = !isConst;
        return;
    }

    // <postfiks_izraz> ::= <postfiks_izraz> L_ZAGRADA D_ZAGRADA
    if (node->children[1]->content.find("L_ZAGRADA") == 0 &&
        node->children[2]->content.find("D_ZAGRADA") == 0)
    {
        // Function call with no arguments
        if (!TypeUtils::isFunctionType(node->children[0]->typeInfo))
        {
            reportError(node);
            return;
        }

        // Check if function accepts void
        if (!node->children[0]->typeInfo.isVoidParam())
        {
            reportError(node);
            return;
        }

        node->typeInfo = TypeInfo(node->children[0]->typeInfo.getReturnType());
        node->isLValue = false;
        return;
    }

    // <postfiks_izraz> ::= <postfiks_izraz> L_ZAGRADA <lista_argumenata> D_ZAGRADA
    if (node->children[1]->content.find("L_ZAGRADA") == 0)
    {
        // Function call with arguments
        if (!TypeUtils::isFunctionType(node->children[0]->typeInfo))
        {
            reportError(node);
            return;
        }

        process_lista_argumenata(node->children[2]);

        // Check parameter compatibility
        const auto &params = node->children[0]->typeInfo.getFunctionParams();
        const auto &args = node->children[2]->typeInfo.getFunctionParams();

        if (params.size() != args.size())
        {
            reportError(node);
            return;
        }

        for (size_t i = 0; i < params.size(); i++)
        {
            if (!args[i].canImplicitlyConvertTo(params[i]))
            {
                reportError(node);
                return;
            }
        }

        node->typeInfo = TypeInfo(node->children[0]->typeInfo.getReturnType());
        node->isLValue = false;
        return;
    }

    // <postfiks_izraz> ::= <postfiks_izraz> OP_INC/OP_DEC
    if (node->children[1]->content.find("OP_INC") == 0 ||
        node->children[1]->content.find("OP_DEC") == 0)
    {
        // Check if operand is l-value and numeric type
        if (!node->children[0]->isLValue ||
            !node->children[0]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_lista_argumenata(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <lista_argumenata> ::= <izraz_pridruzivanja>
    if (node->children.size() == 1)
    {
        process_izraz_pridruzivanja(node->children[0]);
        // Create a vector with single argument type
        vector<TypeInfo> params = {node->children[0]->typeInfo};
        node->typeInfo = TypeInfo(BasicType::VOID, params);
        return;
    }

    // <lista_argumenata> ::= <lista_argumenata> ZAREZ <izraz_pridruzivanja>
    if (node->children.size() == 3)
    {
        process_lista_argumenata(node->children[0]);
        process_izraz_pridruzivanja(node->children[2]);

        // Get existing params and add new one
        vector<TypeInfo> params = node->children[0]->typeInfo.getFunctionParams();
        params.push_back(node->children[2]->typeInfo);
        node->typeInfo = TypeInfo(BasicType::VOID, params);
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_unarni_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <unarni_izraz> ::= <postfiks_izraz>
    if (node->children.size() == 1)
    {
        process_postfiks_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <unarni_izraz> ::= (OP_INC | OP_DEC) <unarni_izraz>
    if (node->children[0]->content.find("OP_INC") == 0 ||
        node->children[0]->content.find("OP_DEC") == 0)
    {
        process_unarni_izraz(node->children[1]);

        // Check if operand is l-value and numeric type
        if (!node->children[1]->isLValue ||
            !node->children[1]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    // <unarni_izraz> ::= <unarni_operator> <cast_izraz>
    if (node->children.size() == 2)
    {
        process_cast_izraz(node->children[1]);

        // Check if operand can be converted to int
        if (!node->children[1]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_unarni_operator(Node *node)
{
    // Note: According to lab instructions, no semantic rules need to be checked for unary operators
    // They just generate PLUS, MINUS, OP_TILDA and OP_NEG operators
    // The semantic checking is done in process_unarni_izraz where we ensure operands are of correct type

    // However we should still validate the node structure
    if (!node || node->children.size() != 1)
    {
        reportError(node);
        return;
    }

    string op = node->children[0]->content;
    if (op.find("PLUS") != 0 &&
        op.find("MINUS") != 0 &&
        op.find("OP_TILDA") != 0 &&
        op.find("OP_NEG") != 0)
    {
        reportError(node);
    }
}

void SemanticAnalyzer::IzrazProcessor::process_cast_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <cast_izraz> ::= <unarni_izraz>
    if (node->children.size() == 1)
    {
        process_unarni_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <cast_izraz> ::= L_ZAGRADA <ime_tipa> D_ZAGRADA <cast_izraz>
    if (node->children.size() == 4)
    {
        // Process both sides
        process_ime_tipa(node->children[1]);
        process_cast_izraz(node->children[3]);

        // Get target type from ime_tipa
        TypeInfo targetType = node->children[1]->typeInfo;
        TypeInfo sourceType = node->children[3]->typeInfo;

        // Rules:
        // 1. Cannot cast from function type
        if (TypeUtils::isFunctionType(sourceType))
        {
            // Only report error if the final cast introduces a problem
            if (node->children[3]->symbol == "<unarni_izraz>")
            {
                reportError(node);
                return;
            }
        }

        // 2. Target cannot be void
        if (targetType.isVoid())
        {
            reportError(node);
            return;
        }

        // 3. Cast is only allowed for numeric types
        if (!TypeUtils::isNumericType(sourceType) &&
            !TypeUtils::isNumericType(targetType))
        {
            reportError(node);
            return;
        }

        // Set type and l-value status
        node->typeInfo = targetType;
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_ime_tipa(Node *node)
{
    cerr << "DEBUG: Processing ime_tipa" << endl;
    if (!node)
    {
        reportError(node);
        return;
    }

    // <ime_tipa> ::= <specifikator_tipa>
    if (node->children.size() == 1)
    {
        process_specifikator_tipa(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        cerr << "DEBUG: ime_tipa type set to: " << node->typeInfo.toString() << endl;
        return;
    }

    // <ime_tipa> ::= KR_CONST <specifikator_tipa>
    if (node->children.size() == 2)
    {
        if (node->children[0]->content.find("KR_CONST") != 0)
        {
            reportError(node);
            return;
        }

        process_specifikator_tipa(node->children[1]);

        // Cannot have const void
        if (node->children[1]->typeInfo.isVoid())
        {
            reportError(node);
            return;
        }

        // Create const-qualified type
        node->typeInfo = TypeInfo(node->children[1]->typeInfo.getBaseType(), true);
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_specifikator_tipa(Node *node)
{
    if (!node || node->children.size() != 1)
    {
        reportError(node);
        return;
    }

    string typeSpecifier = node->children[0]->content;

    // <specifikator_tipa> ::= KR_VOID
    if (typeSpecifier.find("KR_VOID") == 0)
    {
        node->typeInfo = TypeInfo(BasicType::VOID);
        return;
    }
    // <specifikator_tipa> ::= KR_CHAR
    else if (typeSpecifier.find("KR_CHAR") == 0)
    {
        node->typeInfo = TypeInfo(BasicType::CHAR);
        return;
    }
    // <specifikator_tipa> ::= KR_INT
    else if (typeSpecifier.find("KR_INT") == 0)
    {
        node->typeInfo = TypeInfo(BasicType::INT);
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_multiplikativni_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <multiplikativni_izraz> ::= <cast_izraz>
    if (node->children.size() == 1)
    {
        process_cast_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <multiplikativni_izraz> ::= <multiplikativni_izraz> (OP_PUTA | OP_DIJELI | OP_MOD) <cast_izraz>
    if (node->children.size() == 3)
    {
        // Process the operands
        process_multiplikativni_izraz(node->children[0]);
        process_cast_izraz(node->children[2]);

        // Both operands must be implicitly convertible to int
        if (!node->children[0]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)) ||
            !node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        // Check if the operator is valid
        string op = node->children[1]->content;
        if (op.find("OP_PUTA") != 0 &&
            op.find("OP_DIJELI") != 0 &&
            op.find("OP_MOD") != 0)
        {
            reportError(node);
            return;
        }

        // Result is always int and never an l-value
        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_aditivni_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <aditivni_izraz> ::= <multiplikativni_izraz>
    if (node->children.size() == 1)
    {
        process_multiplikativni_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <aditivni_izraz> ::= <aditivni_izraz> (PLUS | MINUS) <multiplikativni_izraz>
    if (node->children.size() == 3)
    {
        // Process the operands
        process_aditivni_izraz(node->children[0]);
        process_multiplikativni_izraz(node->children[2]);

        // Both operands must be implicitly convertible to int
        if (!node->children[0]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)) ||
            !node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        // Check if the operator is valid
        string op = node->children[1]->content;
        if (op.find("PLUS") != 0 && op.find("MINUS") != 0)
        {
            reportError(node);
            return;
        }

        // Result is always int and never an l-value
        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_odnosni_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <odnosni_izraz> ::= <aditivni_izraz>
    if (node->children.size() == 1)
    {
        process_aditivni_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <odnosni_izraz> ::= <odnosni_izraz> (OP_LT | OP_GT | OP_LTE | OP_GTE) <aditivni_izraz>
    if (node->children.size() == 3)
    {
        // Process both operands
        process_odnosni_izraz(node->children[0]);
        process_aditivni_izraz(node->children[2]);

        // Both operands must be implicitly convertible to int
        if (!node->children[0]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)) ||
            !node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        // Validate operator
        string op = node->children[1]->content;
        if (op.find("OP_LT") != 0 &&
            op.find("OP_GT") != 0 &&
            op.find("OP_LTE") != 0 &&
            op.find("OP_GTE") != 0)
        {
            reportError(node);
            return;
        }

        // Result is int (representing boolean) and never an l-value
        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_jednakosni_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <jednakosni_izraz> ::= <odnosni_izraz>
    if (node->children.size() == 1)
    {
        process_odnosni_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <jednakosni_izraz> ::= <jednakosni_izraz> (OP_EQ | OP_NEQ) <odnosni_izraz>
    if (node->children.size() == 3)
    {
        // Process both operands
        process_jednakosni_izraz(node->children[0]);
        process_odnosni_izraz(node->children[2]);

        // Both operands must be implicitly convertible to int
        if (!node->children[0]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)) ||
            !node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        // Validate operator
        string op = node->children[1]->content;
        if (op.find("OP_EQ") != 0 && op.find("OP_NEQ") != 0)
        {
            reportError(node);
            return;
        }

        // Result is int (representing boolean) and never an l-value
        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_bin_i_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <bin_i_izraz> ::= <jednakosni_izraz>
    if (node->children.size() == 1)
    {
        process_jednakosni_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <bin_i_izraz> ::= <bin_i_izraz> OP_BIN_I <jednakosni_izraz>
    if (node->children.size() == 3)
    {
        process_bin_i_izraz(node->children[0]);
        process_jednakosni_izraz(node->children[2]);

        // Both operands must be implicitly convertible to int
        if (!node->children[0]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)) ||
            !node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        // Result is int and never an l-value
        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_bin_xili_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <bin_xili_izraz> ::= <bin_i_izraz>
    if (node->children.size() == 1)
    {
        process_bin_i_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <bin_xili_izraz> ::= <bin_xili_izraz> OP_BIN_XILI <bin_i_izraz>
    if (node->children.size() == 3)
    {
        process_bin_xili_izraz(node->children[0]);
        process_bin_i_izraz(node->children[2]);

        // Both operands must be implicitly convertible to int
        if (!node->children[0]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)) ||
            !node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        // Result is int and never an l-value
        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_bin_ili_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <bin_ili_izraz> ::= <bin_xili_izraz>
    if (node->children.size() == 1)
    {
        process_bin_xili_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <bin_ili_izraz> ::= <bin_ili_izraz> OP_BIN_ILI <bin_xili_izraz>
    if (node->children.size() == 3)
    {
        process_bin_ili_izraz(node->children[0]);
        process_bin_xili_izraz(node->children[2]);

        // Both operands must be implicitly convertible to int
        if (!node->children[0]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)) ||
            !node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        // Result is int and never an l-value
        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_log_i_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <log_i_izraz> ::= <bin_ili_izraz>
    if (node->children.size() == 1)
    {
        process_bin_ili_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <log_i_izraz> ::= <log_i_izraz> OP_I <bin_ili_izraz>
    if (node->children.size() == 3)
    {
        process_log_i_izraz(node->children[0]);
        process_bin_ili_izraz(node->children[2]);

        // Both operands must be implicitly convertible to int
        if (!node->children[0]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)) ||
            !node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        // Result is int and never an l-value
        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_log_ili_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <log_ili_izraz> ::= <log_i_izraz>
    if (node->children.size() == 1)
    {
        process_log_i_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <log_ili_izraz> ::= <log_ili_izraz> OP_ILI <log_i_izraz>
    if (node->children.size() == 3)
    {
        process_log_ili_izraz(node->children[0]);
        process_log_i_izraz(node->children[2]);

        // Both operands must be implicitly convertible to int
        if (!node->children[0]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)) ||
            !node->children[2]->typeInfo.canImplicitlyConvertTo(TypeInfo(BasicType::INT)))
        {
            reportError(node);
            return;
        }

        // Result is int and never an l-value
        node->typeInfo = TypeInfo(BasicType::INT);
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_izraz_pridruzivanja(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // <izraz_pridruzivanja> ::= <log_ili_izraz>
    if (node->children.size() == 1)
    {
        process_log_ili_izraz(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
        return;
    }

    // <izraz_pridruzivanja> ::= <postfiks_izraz> OP_PRIDRUZI <izraz_pridruzivanja>
    if (node->children.size() == 3)
    {
        // Process both sides of the assignment
        process_postfiks_izraz(node->children[0]);
        process_izraz_pridruzivanja(node->children[2]);

        // Check if left side is an l-value
        if (!node->children[0]->isLValue)
        {
            reportError(node);
            return;
        }

        // Validate operator
        if (node->children[1]->content.find("OP_PRIDRUZI") != 0)
        {
            reportError(node);
            return;
        }

        // Check type compatibility for assignment
        // Right side must be implicitly convertible to left side's type
        if (!node->children[2]->typeInfo.canImplicitlyConvertTo(node->children[0]->typeInfo))
        {
            reportError(node);
            return;
        }

        // The type of the assignment expression is the type of the left operand
        // but it's not an l-value
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = false;
        return;
    }

    reportError(node);
}

void SemanticAnalyzer::IzrazProcessor::process_izraz(Node *node)
{
    if (!node)
    {
        reportError(node);
        return;
    }

    // According to 4.4.4, <izraz> can be either:
    // <izraz> ::= <izraz_pridruzivanja>
    // <izraz> ::= <izraz> ZAREZ <izraz_pridruzivanja>

    if (node->children.size() == 1)
    {
        // <izraz> ::= <izraz_pridruzivanja>
        process_izraz_pridruzivanja(node->children[0]);
        node->typeInfo = node->children[0]->typeInfo;
        node->isLValue = node->children[0]->isLValue;
    }
    else if (node->children.size() == 3)
    {
        // <izraz> ::= <izraz> ZAREZ <izraz_pridruzivanja>
        process_izraz(node->children[0]);
        process_izraz_pridruzivanja(node->children[2]);
        node->typeInfo = node->children[2]->typeInfo; // Type of the last expression
        node->isLValue = false;
    }
    else
    {
        reportError(node);
    }
}