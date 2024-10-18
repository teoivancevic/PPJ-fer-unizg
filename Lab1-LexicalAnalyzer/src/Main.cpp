#include"automata.hpp"

using std::cin;
using std::cout;

int main () {

    Regex oZ = "0|1|2|3|4|5|6|7";
    oZ.save_as("{oktalnaZnamenka}");
    Regex dZ = "{oktalnaZnamenka}|8|9";
    dZ.save_as("{dekadskaZnamenka}");
    Regex hz = "a|b|c|d|e|f|{dekadskaZnamenka}|A|B|C|D|E|F";
    hz.save_as("{heksadekadskaZnamenka}");

    cout <<hz;
}