#include"Regex.hpp"

using namespace print_util;

int main () {
    const std::string str = 
        "(((w)*(((h)))(||||)y|did(the()chi*((ck*{en}*|cros****s)*)th**e)ro(((()())((()))()))**ad?{to see}w||hat(((s)(on|||t|he)))*oth**er(sid{e})))";
    Regex reg = str;

    //pojednostavljeno
    std::cout <<reg.reduce() <<"\n\n";

    //stablasti ispis
    printRegex(reg);
    
    std::cin.get();
}