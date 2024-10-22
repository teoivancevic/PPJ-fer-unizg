#include"Generator.cpp"
#include<iostream>
#include"Utils.hpp"

using std::cin;
using std::cout;


//ako nađeš bug javi
int main () {

    // Regex reg = R"(\\|\n|\")";
    // cout <<reg;

    Generator g("minus_lang.txt", "tablica.hpp");
    g.generate();
}