#include"eNKA.hpp"
#include<iostream>

using std::cin;
using std::cout;

int main () {

    Regex r = "(((w)*(((h)))(||||)y|did(the()chi*((ck*{en}*|cros****s)*)th**e)ro(((()())((()))()))**ad?{to see}w||hat(((s)(on|||t|he)))*oth**er(sid{e})))";

    FiniteAutomata a = Regex(
        "Your mother|Deez nuts gotem!"
    );

    cout <<a.evaluate("Your mother");
}