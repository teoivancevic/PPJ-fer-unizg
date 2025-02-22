#pragma once

#include "Data.hpp"

class SemanticAnalyzer
{
private:
    class Validator
    {
    public:
        static bool validateNumConstant(const string &unit, TypeInfo &outType)
        {
            try
            {
                int value = stoi(unit);
                if (!Constants::isValidNumConstant(value))
                    return false;

                outType = TypeInfo(BasicType::INT);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        static bool validateCharConstant(const string &unit, TypeInfo &outType)
        {
            string charValue = unit.substr(1, unit.length() - 2); // Remove quotes

            if (charValue.length() == 1)
            {
                if (!Constants::isValidNumConstant(static_cast<int>(charValue[0])))
                    return false;
            }
            else if (charValue.length() == 2 && charValue[0] == '\\')
                if (!Constants::isValidEscapeSequence(charValue[1]))
                    return false;
                else
                    return false;

            outType = TypeInfo(BasicType::CHAR);
            return true;
        }

        static bool validateNizZnakova(const string &unit, TypeInfo &outType)
        {
            for (size_t i = 0; i < unit.length(); i++)
            {
                if (unit[i] == '\\')
                {
                    if (i + 1 >= unit.length())
                    {
                        return false;
                    }
                    if (!Constants::isValidEscapeSequence(unit[i + 1]))
                    {
                        return false;
                    }
                    i++;
                }
                else if (!Constants::isValidNumConstant(static_cast<int>(unit[i])))
                {
                    return false;
                }
            }

            outType = TypeUtils::makeArrayType(BasicType::CHAR, true);
            return true;
        }
    };

    class Processor
    {
    public:
        Processor(SemanticAnalyzer *SA) : SA(SA) {}

        void processAll(Node *node)
        {
            for (Node *child : node->children)
                if (!node->isTerminating())
                    SA->process(child);
        }

    protected:
        SemanticAnalyzer *SA;
        SymbolTable *&currentScope = SA->currentScope;
    };

    // Za obradu izraza (4.4.4)
    class IzrazProcessor : Processor
    {
    public:
        using Processor::Processor;

        void process_primarni_izraz(Node *node);
        void process_postfiks_izraz(Node *node);
        void process_lista_argumenata(Node *node);
        void process_unarni_izraz(Node *node);
        void process_unarni_operator(Node *node);
        void process_cast_izraz(Node *node);
        void process_ime_tipa(Node *node);
        void process_specifikator_tipa(Node *node);
        void process_multiplikativni_izraz(Node *node);
        void process_aditivni_izraz(Node *node);
        void process_odnosni_izraz(Node *node);
        void process_jednakosni_izraz(Node *node);
        void process_bin_i_izraz(Node *node);
        void process_bin_xili_izraz(Node *node);
        void process_bin_ili_izraz(Node *node);
        void process_log_i_izraz(Node *node);
        void process_log_ili_izraz(Node *node);
        void process_izraz_pridruzivanja(Node *node);
        void process_izraz(Node *node);
    };

    // Za obradu programske strukture (4.4.5)
    class NaredbaProcessor : Processor
    {
    public:
        using Processor::Processor;

        void process_slozena_naredba(Node *node);
        void process_lista_naredbi(Node *node);
        void process_naredba(Node *node);
        void process_izraz_naredba(Node *node);
        void process_naredba_grananja(Node *node);
        void process_naredba_petlje(Node *node);
        void process_naredba_skoka(Node *node);
        void process_prijevodna_jedinica(Node *node);
        void process_vanjska_deklaracija(Node *node);
    };

    // Za obradu deklaracija i definicija (4.4.6)
    class DeklaracijaProcessor : Processor
    {
    public:
        using Processor::Processor;

        void process_definicija_funkcije(Node *node);
        void process_lista_parametara(Node *node);
        void process_deklaracija_parametra(Node *node);
        void process_lista_deklaracija(Node *node);
        void process_deklaracija(Node *node);
        void process_lista_init_deklaratora(Node *node);
        void process_init_deklarator(Node *node);
        void process_izravni_deklarator(Node *node);
        void process_inicijalizator(Node *node);
    };

    Node *root;
    SymbolTable *currentScope;

    IzrazProcessor izrazProcessor;
    NaredbaProcessor naredbaProcessor;
    DeklaracijaProcessor deklaracijaProcessor;

public:
    SemanticAnalyzer(Node *rootNode)
        : root(rootNode),
          currentScope(new SymbolTable()),
          izrazProcessor(this),
          naredbaProcessor(this),
          deklaracijaProcessor(this) {}

    void run()
    {
        process(root);

        // Final checks
        checkMainExists();
        checkAllFunctionsDefined();
    }

    // aka provjeri
    void process(Node *node)
    {
        if (node->symbol == "<prijevodna_jedinica>")
        {
            naredbaProcessor.process_prijevodna_jedinica(node);
        }

        else if (node->symbol == "<primarni_izraz>")
        {
            izrazProcessor.process_primarni_izraz(node);
        }
        else if (node->symbol == "<postfiks_izraz>")
        {
            izrazProcessor.process_postfiks_izraz(node);
        }
        else if (node->symbol == "<lista_argumenata>")
        {
            izrazProcessor.process_lista_argumenata(node);
        }
        else if (node->symbol == "<unarni_izraz>")
        {
            izrazProcessor.process_unarni_izraz(node);
        }
        else if (node->symbol == "<unarni_operator>")
        {
            izrazProcessor.process_unarni_operator(node);
        }
        else if (node->symbol == "<cast_izraz>")
        {
            izrazProcessor.process_cast_izraz(node);
        }
        else if (node->symbol == "<ime_tipa>")
        {
            izrazProcessor.process_ime_tipa(node);
        }
        else if (node->symbol == "<specifikator_tipa>")
        {
            izrazProcessor.process_specifikator_tipa(node);
        }
        else if (node->symbol == "<multiplikativni_izraz>")
        {
            izrazProcessor.process_multiplikativni_izraz(node);
        }
        else if (node->symbol == "<aditivni_izraz>")
        {
            izrazProcessor.process_aditivni_izraz(node);
        }
        else if (node->symbol == "<odnosni_izraz>")
        {
            izrazProcessor.process_odnosni_izraz(node);
        }
        else if (node->symbol == "<jednakosni_izraz>")
        {
            izrazProcessor.process_jednakosni_izraz(node);
        }
        else if (node->symbol == "<bin_i_izraz>")
        {
            izrazProcessor.process_bin_i_izraz(node);
        }
        else if (node->symbol == "<bin_xili_izraz>")
        {
            izrazProcessor.process_bin_xili_izraz(node);
        }
        else if (node->symbol == "<bin_ili_izraz>")
        {
            izrazProcessor.process_bin_ili_izraz(node);
        }
        else if (node->symbol == "<log_i_izraz>")
        {
            izrazProcessor.process_log_i_izraz(node);
        }
        else if (node->symbol == "<log_ili_izraz>")
        {
            izrazProcessor.process_log_ili_izraz(node);
        }
        else if (node->symbol == "<izraz_pridruzivanja>")
        {
            izrazProcessor.process_izraz_pridruzivanja(node);
        }
        else if (node->symbol == "<izraz>")
        {
            izrazProcessor.process_izraz(node);
        }

        else if (node->symbol == "<slozena_naredba>")
        {
            naredbaProcessor.process_slozena_naredba(node);
        }
        else if (node->symbol == "<lista_naredbi>")
        {
            naredbaProcessor.process_lista_naredbi(node);
        }
        else if (node->symbol == "<naredba>")
        {
            naredbaProcessor.process_naredba(node);
        }
        else if (node->symbol == "<izraz_naredba>")
        {
            naredbaProcessor.process_izraz_naredba(node);
        }
        else if (node->symbol == "<naredba_grananja>")
        {
            naredbaProcessor.process_naredba_grananja(node);
        }
        else if (node->symbol == "<naredba_petlje>")
        {
            naredbaProcessor.process_naredba_petlje(node);
        }
        else if (node->symbol == "<naredba_skoka>")
        {
            naredbaProcessor.process_naredba_skoka(node);
        }
        else if (node->symbol == "<vanjska_deklaracija>")
        {
            naredbaProcessor.process_vanjska_deklaracija(node);
        }

        else if (node->symbol == "<definicija_funkcije>")
        {
            deklaracijaProcessor.process_definicija_funkcije(node);
        }
        else if (node->symbol == "<lista_parametara>")
        {
            deklaracijaProcessor.process_lista_parametara(node);
        }
        else if (node->symbol == "<deklaracija_parametra>")
        {
            deklaracijaProcessor.process_deklaracija_parametra(node);
        }
        else if (node->symbol == "<lista_deklaracija>")
        {
            deklaracijaProcessor.process_lista_deklaracija(node);
        }
        else if (node->symbol == "<deklaracija>")
        {
            deklaracijaProcessor.process_deklaracija(node);
        }
    }

private:
    void checkMainExists()
    {
        SymbolTableEntry *mainEntry = currentScope->lookup("main");

        if (!mainEntry || !mainEntry->type.isFunc() || mainEntry->type.returnType != BasicType::INT ||
            !mainEntry->type.isVoidParam())
        {
            cout << "main" << endl;
            exit(0);
        }
    }

    void checkAllFunctionsDefined()
    {
        for (const auto &[name, symbol] : currentScope->symbols)
        {
            if (symbol.type.isFunc() && !symbol.isDefined)
            {
                cout << name << endl;
                exit(0);
            }
        }
    }
};
