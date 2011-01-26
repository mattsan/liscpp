#include "parser.h"

#include <sstream>
#include <list>
#include <stdexcept>
#include <cstdlib>

namespace
{

typedef std::list<std::string> Tokens;

const Atom* atom(const std::string& token)
{
    char* endptr;

    int i = std::strtol(token.c_str(), &endptr, 10);
    if(*endptr == 0)
    {
        return new Integer(i);
    }

    double r = std::strtod(token.c_str(), &endptr);
    if(*endptr == 0)
    {
        return new Real(r);
    }

    return new Symbol(token);
}

const Atom* readFrom(Tokens::const_iterator& cur, Tokens::const_iterator end)
{
    if(cur == end)
    {
        throw std::runtime_error("unexpected EOF while reading");
    }

    std::string token = *cur++;

    if(token == "(")
    {
        std::list<const Atom*> atoms;
        for(;;)
        {
            if(*cur == ")")
            {
                ++cur;
                break;
            }
            atoms.push_back(readFrom(cur, end));
        }

        const Node* node = Node::getNull();
        for(std::list<const Atom*>::reverse_iterator i = atoms.rbegin(); i != atoms.rend(); ++i)
        {
            node = new Node(*i, node);
        }
        return node;
    }

    if(token == ")")
    {
        throw std::runtime_error("unexpected");
    }

    return atom(token);
}

Tokens tokenize(const std::string& program)
{
    std::stringstream ss;
    for(std::string::const_iterator i = program.begin(); i != program.end(); ++i)
    {
        if(*i == '(')
        {
            ss << " ( ";
        }
        else if(*i == ')')
        {
            ss << " ) ";
        }
        else
        {
            ss << *i;
        }
    }
    ss << " ";

    Tokens      tokens;
    std::string token;

    while((ss >> token).good())
    {
        tokens.push_back(token);
    }

    return tokens;
}

} // end of anonymous namespace

const Atom* parse(const std::string& program)
{
    Tokens tokens = tokenize(program);
    Tokens::const_iterator begin = tokens.begin();
    return readFrom(begin, tokens.end());
}
