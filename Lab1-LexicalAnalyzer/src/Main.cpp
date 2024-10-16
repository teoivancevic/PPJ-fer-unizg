#include"automata.hpp"
#include<iostream>

using std::cin;
using std::cout;

int main () {

    //Regex r = "(((w)*(((h)))(||||)y|did(the()chi*((ck*{en}*|cros****s)*)th**e)ro(((()())((()))()))**ad?{to see}w||hat(((s)(on|||t|he)))*oth**er(sid{e})))";

    NKA a = Regex(
        "mama*|(ta)*"
    );

    cout <<a.evaluate("mamta");
    cout <<a.evaluate("tatatatatatatatatata");
}