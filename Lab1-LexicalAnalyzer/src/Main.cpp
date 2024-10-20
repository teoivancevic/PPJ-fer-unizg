#include"Generator.cpp"
#include"automata.hpp"
#include<iostream>

using std::cin;
using std::cout;

//ako nađeš bug javi
int main () {

    Generator g("C_lang.txt");
    g.generate();

    NKA test = g.automata["S_string"][0].regex;

    cout <<test.evaluate("\"bababooeyfafafugi\"");
}