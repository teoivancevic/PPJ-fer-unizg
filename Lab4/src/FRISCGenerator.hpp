// FRISCGenerator.hpp
#pragma once

#include "Data.hpp"
#include <fstream>
#include <sstream>

class FRISCGenerator {
private:
    Node* root;
    ofstream outputFile;

    class GeneratorBase {
    protected:
        FRISCGenerator* FG;
        ofstream& output;

    public:
        GeneratorBase(FRISCGenerator* generator) : FG(generator), output(generator->outputFile) {}
        
        void emit(const string& code) {
            output << code << "\n";
        }
    };

    // Similar structure to Lab3, but for generating code
    class IzrazProcessor : public GeneratorBase {
    public:
        using GeneratorBase::GeneratorBase;
        void process_primarni_izraz(Node *node, bool emitCode = true);
        void process_postfiks_izraz(Node *node, bool emitCode = true);
        void process_lista_argumenata(Node *node, bool emitCode = true);
        void process_unarni_izraz(Node *node, bool emitCode = true);
        void process_unarni_operator(Node *node, bool emitCode = true);
        void process_cast_izraz(Node *node, bool emitCode = true);
        void process_ime_tipa(Node *node, bool emitCode = true);
        void process_specifikator_tipa(Node *node, bool emitCode = true);
        void process_multiplikativni_izraz(Node *node, bool emitCode = true);
        void process_aditivni_izraz(Node *node, bool emitCode = true);
        void process_odnosni_izraz(Node *node, bool emitCode = true);
        void process_jednakosni_izraz(Node *node, bool emitCode = true);
        void process_bin_i_izraz(Node *node, bool emitCode = true);
        void process_bin_xili_izraz(Node *node, bool emitCode = true);
        void process_bin_ili_izraz(Node *node, bool emitCode = true);
        void process_log_i_izraz(Node *node, bool emitCode = true);
        void process_log_ili_izraz(Node *node, bool emitCode = true);
        void process_izraz_pridruzivanja(Node *node, bool emitCode = true);
        void process_izraz(Node *node, bool emitCode = true);
    };

    class NaredbaProcessor : public GeneratorBase {
    public:
        using GeneratorBase::GeneratorBase;
        void process_slozena_naredba(Node *node);
        void process_lista_naredbi(Node *node);
        void process_naredba(Node *node);
        void process_izraz_naredba(Node *node);
        void process_naredba_grananja(Node *node);
        void process_naredba_petlje(Node *node);
        void process_naredba_skoka(Node *node);
        
        void process_prijevodna_jedinica(Node *node, bool isFunctionPass = true);
        void process_vanjska_deklaracija(Node *node, bool isFunctionPass);
    };

    class DeklaracijaProcessor : public GeneratorBase {
    public:
        using GeneratorBase::GeneratorBase;
        void process_definicija_funkcije(Node *node);
        void process_lista_parametara(Node *node);
        void process_deklaracija_parametra(Node *node);
        void process_lista_deklaracija(Node *node);
        void process_deklaracija(Node *node);
        void process_lista_init_deklaratora(Node *node);
        void process_init_deklarator(Node *node);
        void process_izravni_deklarator(Node *node);
        void process_inicijalizator(Node *node, bool emitCode = true);
        void process_lista_izraza_pridruzivanja(Node* node);
    };

    
public:
    
    IzrazProcessor izrazProcessor;
    NaredbaProcessor naredbaProcessor;
    DeklaracijaProcessor deklaracijaProcessor;

    FRISCGenerator() : 
        izrazProcessor(this),
        naredbaProcessor(this),
        deklaracijaProcessor(this) {}

    void generate(Node* rootNode) {
        root = rootNode;
        outputFile.open("a.frisc");
        
        // Program header
        emit("    MOVE 40000, R7");
        emit("    CALL F_MAIN");
        emit("    HALT");
        emit("");
        
        // Generate code starting from root
        generateCode(root);
        
        outputFile.close();
    }

private:
    void processPass(Node* node, bool isFunctionPass);
    void generateFunctions(Node* node);
    void generateDeclarations(Node* node);

    void emit(const string& code) {
        outputFile << code << "\n";
    }

    void generateCode(Node* node);
};
