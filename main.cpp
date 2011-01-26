#include "atoms.h"
#include "parser.h"
#include "functions.h"

#include <iostream>
#include <string>

void repl(const std::string& prompt, Env& env)
{
    std::cout << prompt << std::flush;
    std::string s;
    while(std::getline(std::cin, s).good())
    {
        const Atom* atom = parse(s);
        std::cout << *atom << " -> " << *atom->eval(env) << std::endl;
        std::cout << prompt << std::flush;
    }
    std::cout << std::endl;
}

int main(int, char* [])
{
    Env env;

    appendFunctions(env);

    repl("lis.cpp> ", env);

    Atom::releaseAll();

    return 0;
}
