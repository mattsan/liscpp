#include "atoms.h"

#include <algorithm>
#include <stdexcept>
#include <sstream>

std::list<const Atom*> Atom::pool_;

class Env::Matcher
{
public:
    Matcher(const std::string& key) : key_(key) {}

    bool operator () (const std::pair<std::string, const Atom*>& i)
    {
        return i.first == key_;
    }

private:
    const std::string& key_;
};

void Env::push(const std::string& key, const Atom* value)
{
    dictionary_.push_front(std::make_pair(key, value));
}

void Env::pop()
{
    dictionary_.pop_front();
}

const Atom* Env::find(const std::string& key) const
{
    Dictionary::const_iterator i = std::find_if(dictionary_.begin(), dictionary_.end(), Matcher(key));
    if(i == dictionary_.end())
    {
        throw std::runtime_error(key + " is not defined");
    }
    return i->second;
}

void Atom::releaseAll()
{
    struct _ { static void delete_(const Atom* atom) { delete atom; } };

    std::for_each(pool_.begin(), pool_.end(), _::delete_);
}

void Atom::assert_(bool cond, const std::string& message)
{
    if( ! cond)
    {
        throw std::runtime_error(message);
    }
}

Atom::Atom()
{
}

Atom::~Atom()
{
}


const Atom* Atom::evalList(const Node* node, Env& env) const
{
    std::stringstream ss;
    ss << *node;
    throw std::runtime_error("cannot eval " + ss.str());
}

std::ostream& operator << (std::ostream& out, const Atom& atom)
{
    atom.write(out);
    return out;
}

Integer::Integer(int i) : i_(i)
{
    pool_.push_back(this);
}

void Integer::write(std::ostream& out) const
{
    out << i_;
}

const Atom* Integer::eval(Env& env) const
{
    return this;
}

int Integer::value() const
{
    return i_;
}

Real::Real(double r) : r_(r)
{
    pool_.push_back(this);
}

void Real::write(std::ostream& out) const
{
    out << r_;
}

const Real* Real::eval(Env& env) const
{
    return this;
}

double Real::value() const
{
    return r_;
}

Bool::Bool(bool b) : b_(b)
{
    pool_.push_back(this);
}

void Bool::write(std::ostream& out) const
{
    out << std::boolalpha << b_;
}

const Bool* Bool::eval(Env& env) const
{
    return this;
}

bool Bool::value() const
{
    return b_;
}

Symbol::Symbol(const std::string& s) : s_(s)
{
    pool_.push_back(this);
}

void Symbol::write(std::ostream& out) const
{
    out << s_;
}

const Atom* Symbol::eval(Env& env) const
{
    return env.find(s_);
}

const Atom* Symbol::evalList(const Node* node, Env& env) const
{
    return node->evalSymbol(this, env);
}

const std::string& Symbol::value() const
{
    return s_;
}

Function::Function(const Node* args) : args_(args)
{
}

void Function::write(std::ostream& out) const
{
    out << "primitive function";
}

const Atom* Function::evalList(const Node* node, Env& env) const
{
    return node->evalFunction(this, env);
}

const Node* Function::args() const
{
    return args_;
}

Lambda::Lambda(const Node* args, const Node* exp) : Function(args), exp_(exp)
{
    pool_.push_back(this);
}

void Lambda::write(std::ostream& out) const
{
    out << "lambda " << *args() << " " << *exp_;
}

const Atom* Lambda::eval(Env& env) const
{
    return exp_->eval(env);
}


Node::Iterator::Iterator(const Node* node) : node_(node)
{
}

bool Node::Iterator::good() const
{
    return (node_ != 0) && (node_ != Node::getNull());
}

const Node* Node::Iterator::operator * () const
{
    Atom::assert_(node_, "Node is null");
    return node_;
}

const Node* Node::Iterator::operator -> () const
{
    Atom::assert_(node_, "Node is null");
    return node_;
}

Node::Iterator& Node::Iterator::operator ++ ()
{
    node_ = node_->cdr_;
    return *this;
}

Node::Iterator Node::Iterator::operator ++ (int)
{
    Iterator result(*this);
    node_ = node_->cdr_;
    return result;
}

const Node* Node::getNull()
{
    static const Node* null = new Node;
    return null;
}

Node::Node() : car_(0), cdr_(0)
{
    pool_.push_back(this);
}

Node::Node(const Atom* car, const Node* cdr) : car_(car), cdr_(cdr)
{
    assert_(car, "atom is null");
    pool_.push_back(this);
}

void Node::write(std::ostream& out) const
{
    out << "( ";
    for(const Node* i = this; i->car_ != 0; i = i->cdr_)
    {
        out << *i->car_ << " ";
    }
    out << ")";
}

const Atom* Node::car() const
{
    assert_(car_, "car is null");
    return car_;
}

const Node* Node::cdr() const
{
    return cdr_;
}

const Atom* Node::eval(Env& env) const
{
    if(this == getNull())
    {
        return this;
    }
    Node::Iterator i(this);
    const Node* head = *i++;
    return head->car()->evalList(*i, env);
}

const Atom* Node::evalSymbol(const Symbol* symbol, Env& env) const
{
    Node::Iterator i(this);
    if     (symbol->value() == "quote")  return evalQuote(i, env);
    else if(symbol->value() == "if")     return evalIf(i, env);
    else if(symbol->value() == "set!")   return evalSet(i, env);
    else if(symbol->value() == "define") return evalDefine(i, env);
    else if(symbol->value() == "lambda") return evalLambda(i, env);
    else if(symbol->value() == "begin")  return evalBegin(i, env);
    else                                 return symbol->eval(env)->evalList(this, env);
}

const Atom* Node::evalQuote(Node::Iterator i, Env&) const
{
    return i->car();
}

const Atom* Node::evalIf(Node::Iterator i, Env& env) const
{
    const Atom* test   = (i++)->car();
    const Atom* conseq = (i++)->car();
    const Atom* alt    = (i++)->car();
    const Bool* cond   = test->eval(env)->as<Bool>();

    return (cond->value() ? conseq : alt)->eval(env);
}

const Atom* Node::evalSet(Node::Iterator i, Env& env) const
{
    const Symbol* s     = (i++)->car()->as<Symbol>();
    const Atom*   value = (i++)->car()->eval(env);
    env.find(s->value());
    env.push(s->value(), value);
    return value;
}

const Atom* Node::evalDefine(Node::Iterator i, Env& env) const
{
    const Symbol* s     = (i++)->car()->as<Symbol>();
    const Atom*   value = (i++)->car()->eval(env);
    env.push(s->value(), value);
    return value;
}

const Atom* Node::evalLambda(Node::Iterator i, Env& env) const
{
    const Node* args = (i++)->car()->as<Node>();
    const Node* exp  = (i++)->car()->as<Node>();
    return new Lambda(args, exp);
}

const Atom* Node::evalBegin(Node::Iterator i, Env& env) const
{
    const Atom* result = 0;
    for(; i.good(); ++i)
    {
        result = i->car()->eval(env);
    }
    return result;
}

const Atom* Node::evalFunction(const Function* fun, Env& env) const
{
    int i = 0;
    const Node* args = fun->args();
    for(Node::Iterator a(args), v(this); a.good(); ++a, ++v)
    {
        ++i;
        const std::string& s = a->car()->as<Symbol>()->value();
        if(s == " ")
        {
            env.push(s, *v);
            break;
        }
        env.push(s, v->car()->eval(env));
    }
    const Atom* result = fun->eval(env);
    while(i > 0)
    {
        env.pop();
        --i;
    }

    return result;
}
