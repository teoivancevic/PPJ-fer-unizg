#include"Generator.cpp"
#include<iostream>
#include"Utils.hpp"
//#include"tablica.hpp"

using std::cin;
using std::cout;

using namespace resources;

//ako nađeš bug javi
int main () {

    Regex reg = R"(\\|\n|\")";
    cout <<reg;

    //init();

    cin.get();

    Generator g("minus_lang.txt", "tablica.hpp");
    g.generate();
}