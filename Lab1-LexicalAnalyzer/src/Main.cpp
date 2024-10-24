#include"Generator.cpp"
#include<iostream>
#include"Utils.hpp"
#include"tablica.hpp"

using std::cin;
using std::cout;

using namespace resources;

//za generiranje tablica.hpp pokreni naredbe ispod
//inicijaliziraj tablicu s resources::init()
//ako nađeš bug javi
int main () {
    Generator g("IME_DATOTEKE.txt", "tablica.hpp");
    g.generate();
}