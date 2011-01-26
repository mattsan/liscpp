#include "functions.h"

#include <string>
#include <stdexcept>
#include <functional>

namespace
{

const Node* creatArgList(const std::string& arg1)
{
    return new Node(new Symbol(arg1), 0);
}

const Node* creatArgList(const std::string& arg1, const std::string& arg2)
{
    return new Node(new Symbol(arg1), new Node(new Symbol(arg2), 0));
}

template<typename T> const Atom* newAtom(T);
template<> const Atom* newAtom(int i)    { return new Integer(i); }
template<> const Atom* newAtom(double r) { return new Real(r);    }
template<> const Atom* newAtom(bool b)   { return new Bool(b);    }

template<template<class> class OP>
const Atom* operate(const Atom* lhs, const Atom* rhs)
{
    const Real*    r1 = dynamic_cast<const Real*>(lhs);
    const Real*    r2 = dynamic_cast<const Real*>(rhs);
    const Integer* i1 = dynamic_cast<const Integer*>(lhs);
    const Integer* i2 = dynamic_cast<const Integer*>(rhs);

    if(i1 != 0)
    {
        if(i2 != 0)
        {
            return newAtom(OP<int>()(i1->value(), i2->value()));
        }
        else if(r2 != 0)
        {
            return newAtom(OP<double>()(i1->value(), r2->value()));
        }
        else
        {
            throw std::runtime_error("invalid 2nd argument");
        }
    }
    else if(r1 != 0)
    {
        if(i2 != 0)
        {
            return newAtom(OP<double>()(r1->value(), i2->value()));
        }
        else if(r2 != 0)
        {
            return newAtom(OP<double>()(r1->value(), r2->value()));
        }
        else
        {
            throw std::runtime_error("invalid 2nd argument");
        }
    }
    else
    {
        throw std::runtime_error("invalid 1st argument");
    }
}

class Plus : public Function
{
public:
    Plus() : Function(creatArgList(" x", " y")) { pool_.push_back(this); }

    const Atom* eval(Env& env) const
    {
        const Atom* lhs = env.find(" x")->eval(env);
        const Atom* rhs = env.find(" y")->eval(env);
        return operate<std::plus>(lhs, rhs);
    }
};

class Minus : public Function
{
public:
    Minus() : Function(creatArgList(" x", " y")) { pool_.push_back(this); }

    const Atom* eval(Env& env) const
    {
        const Atom* lhs = env.find(" x")->eval(env);
        const Atom* rhs = env.find(" y")->eval(env);
        return operate<std::minus>(lhs, rhs);
    }
};

class Multiplies : public Function
{
public:
    Multiplies() : Function(creatArgList(" x", " y")) { pool_.push_back(this); }

    const Atom* eval(Env& env) const
    {
        const Atom* lhs = env.find(" x")->eval(env);
        const Atom* rhs = env.find(" y")->eval(env);
        return operate<std::multiplies>(lhs, rhs);
    }
};

class Divides : public Function
{
public:
    Divides() : Function(creatArgList(" x", " y")) { pool_.push_back(this); }

    const Atom* eval(Env& env) const
    {
        const Atom* lhs = env.find(" x")->eval(env);
        const Atom* rhs = env.find(" y")->eval(env);
        return operate<std::divides>(lhs, rhs);
    }
};

class Not : public Function
{
public:
    Not() : Function(creatArgList(" x")) { pool_.push_back(this); }

    const Atom* eval(Env& env) const
    {
        const Bool* operand = env.find(" x")->eval(env)->as<Bool>();
        return new Bool(! operand->value());
    }
};

class Greater : public Function
{
public:
    Greater() : Function(creatArgList(" x", " y")) { pool_.push_back(this); }

    const Atom* eval(Env& env) const
    {
        const Atom* lhs = env.find(" x")->eval(env);
        const Atom* rhs = env.find(" y")->eval(env);
        return operate<std::greater>(lhs, rhs);
    }
};

class Less : public Function
{
public:
    Less() : Function(creatArgList(" x", " y")) { pool_.push_back(this); }

    const Atom* eval(Env& env) const
    {
        const Atom* lhs = env.find(" x")->eval(env);
        const Atom* rhs = env.find(" y")->eval(env);
        return operate<std::less>(lhs, rhs);
    }
};

class GreaterEqual : public Function
{
public:
    GreaterEqual() : Function(creatArgList(" x", " y")) { pool_.push_back(this); }

    const Atom* eval(Env& env) const
    {
        const Atom* lhs = env.find(" x")->eval(env);
        const Atom* rhs = env.find(" y")->eval(env);
        return operate<std::greater_equal>(lhs, rhs);
    }
};

class LessEqual : public Function
{
public:
    LessEqual() : Function(creatArgList(" x", " y")) { pool_.push_back(this); }

    const Atom* eval(Env& env) const
    {
        const Atom* lhs = env.find(" x")->eval(env);
        const Atom* rhs = env.find(" y")->eval(env);
        return operate<std::less_equal>(lhs, rhs);
    }
};

class Equal : public Function
{
public:
    Equal() : Function(creatArgList(" x", " y")) { pool_.push_back(this); }

    const Atom* eval(Env& env) const
    {
        const Atom* lhs = env.find(" x")->eval(env);
        const Atom* rhs = env.find(" y")->eval(env);
        return operate<std::equal_to>(lhs, rhs);
    }
};

class Length : public Function
{
public:
    Length() : Function(creatArgList(" x")) { pool_.push_back(this); }

    const Atom* eval(Env& env) const
    {
        Node::Iterator n(env.find(" x")->eval(env)->as<Node>());
        int i = 0;
        while(n.good())
        {
            ++i;
            ++n;
        }
        return new Integer(i);
    }
};

class Cons : public Function
{
public:
    Cons() : Function(creatArgList(" x", " y")) { pool_.push_back(this); }

    const Atom* eval(Env& env) const
    {
        const Atom* car = env.find(" x");
        const Node* cdr = env.find(" y")->as<Node>();
        return new Node(car, cdr);
    }
};

class Car : public Function
{
public:
    Car() : Function(creatArgList(" x")) { pool_.push_back(this); }

    const Atom* eval(Env& env) const
    {
        return env.find(" x")->as<Node>()->car();
    }
};

class Cdr : public Function
{
public:
    Cdr() : Function(creatArgList(" x")) { pool_.push_back(this); }

    const Atom* eval(Env& env) const
    {
        return env.find(" x")->as<Node>()->cdr();
    }
};

class Append : public Function
{
public:
    Append() : Function(creatArgList(" x", " y")) { pool_.push_back(this); }

    const Atom* eval(Env& env) const
    {
        const Node* lhs = env.find(" x")->eval(env)->as<Node>();
        const Node* rhs = env.find(" y")->eval(env)->as<Node>();
        return new Node(lhs->car(), append(lhs->cdr(), rhs));
    }

private:
    const Node* append(const Node* lhs, const Node* rhs) const
    {
        if(lhs->cdr() == 0)
        {
            return new Node(lhs->car(), rhs);
        }
        else
        {
            return new Node(lhs->car(), append(lhs->cdr(), rhs));
        }
    }
};

class List : public Function
{
public:
    List() : Function(creatArgList(" ")) { pool_.push_back(this); }

    const Atom* eval(Env& env) const
    {
        return eval_(env.find(" ")->as<Node>(), env);
    }

private:
    const Node* eval_(const Node* node, Env& env) const
    {
        if(node == Node::getNull())
        {
            return node;
        }
        else
        {
            return new Node(node->car()->eval(env), eval_(node->cdr(), env));
        }
    }
};

class IsList : public Function
{
public:
    IsList() : Function(creatArgList(" x")) { pool_.push_back(this); }

    const Atom* eval(Env& env) const
    {
        const Node* x = dynamic_cast<const Node*>(env.find(" x"));
        return new Bool(x != 0);
    }
};

class IsNull : public Function
{
public:
    IsNull() : Function(creatArgList(" x")) { pool_.push_back(this); }

    const Atom* eval(Env& env) const
    {
        const Atom* atom = env.find(" x");
        return new Bool((atom == 0) || (atom == Node::getNull()));
    }
};

class IsSymbol : public Function
{
public:
    IsSymbol() : Function(creatArgList(" x")) { pool_.push_back(this); }

    const Atom* eval(Env& env) const
    {
        const Symbol* x = dynamic_cast<const Symbol*>(env.find(" x")->eval(env));
        return new Bool(x != 0);
    }
};

} // end of anonymous namespace

void appendFunctions(Env& env)
{
    env.push("+",       new Plus);
    env.push("-",       new Minus);
    env.push("*",       new Multiplies);
    env.push("/",       new Divides);
    env.push("not",     new Not);
    env.push(">",       new Greater);
    env.push("<",       new Less);
    env.push(">=",      new GreaterEqual);
    env.push("<=",      new LessEqual);
    env.push("=",       new Equal);
    env.push("equal?",  new Equal);
    env.push("length",  new Length);
    env.push("cons",    new Cons);
    env.push("car",     new Car);
    env.push("cdr",     new Cdr);
    env.push("append",  new Append);
    env.push("list",    new List);
    env.push("list?",   new IsList);
    env.push("null?",   new IsNull);
    env.push("symbol?", new IsSymbol);
}
