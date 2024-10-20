#include"Generator.cpp"
#include"automata.hpp"
#include<iostream>

using std::cin;
using std::cout;

int main () {

    Generator g("minus_lang.txt");
    g.generate();

    NKA test = g.automata[g.states[0]][0].regex;

    cout <<test.evaluate("\t");
}