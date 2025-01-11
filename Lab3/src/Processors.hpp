#include "Data.hpp"

class SemanticAnalyzer;

class Validator
{
public:
    static bool validateNumConstant(const string &lexeme, TypeInfo &outType)
    {
        try
        {
            int value = stoi(lexeme);
            if (!Constants::isValidNumConstant(value))
            {
                return false;
            }
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

    static bool validateNizZnakova(const string &lexeme, TypeInfo &outType)
    {
        for (size_t i = 0; i < lexeme.length(); i++)
        {
            if (lexeme[i] == '\\')
            {
                if (i + 1 >= lexeme.length())
                {
                    return false;
                }
                if (!Constants::isValidEscapeSequence(lexeme[i + 1]))
                {
                    return false;
                }
                i++;
            }
            else if (!Constants::isValidNumConstant(static_cast<int>(lexeme[i])))
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
