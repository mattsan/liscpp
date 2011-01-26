#ifndef ATOMS_H
#define ATOMS_H

#include <iosfwd>
#include <list>
#include <string>

class Atom;

class Env
{
public:
    typedef std::list<std::pair<std::string, const Atom*> > Dictionary;

    void push(const std::string& key, const Atom* value);
    void pop();
    const Atom* find(const std::string& key) const;

private:
    class Matcher;

    Dictionary dictionary_;
};

class Node;

class Atom
{
public:
    Atom();
    virtual ~Atom();
    virtual void write(std::ostream& out) const = 0;
    virtual const Atom* eval(Env& env) const = 0;
    virtual const Atom* evalList(const Node* node, Env& env) const;

    static void releaseAll();

protected:
    static void assert_(bool cond, const std::string& message);
    template<class T> const T* as() const;

    static std::list<const Atom*> pool_;
};

std::ostream& operator << (std::ostream& out, const Atom& atom);

template<class T>
const T* Atom::as() const
{
    const T* result = dynamic_cast<const T*>(this);
    assert_(result, "cannot cast");
    return result;
}

class Integer : public Atom
{
public:
    Integer(int i);
    void write(std::ostream& out) const;
    const Atom* eval(Env& env) const;
    int value() const;

private:
    const int i_;
};

class Real : public Atom
{
public:
    Real(double r);
    void write(std::ostream& out) const;
    const Real* eval(Env& env) const;
    double value() const;

private:
    const double r_;
};

class Bool : public Atom
{
public:
    Bool(bool b);
    void write(std::ostream& out) const;
    const Bool* eval(Env& env) const;
    bool value() const;

private:
    const bool b_;
};

class Symbol : public Atom
{
public:
    Symbol(const std::string& s);
    void write(std::ostream& out) const;
    const Atom* eval(Env& env) const;
    const Atom* evalList(const Node* node, Env& env) const;
    const std::string& value() const;

private:
    const std::string s_;
};

class Function : public Atom
{
public:
    Function(const Node* args);
    void write(std::ostream& out) const;
    const Atom* evalList(const Node* node, Env& env) const;
    const Node* args() const;

private:
    const Node* args_;
};

class Lambda : public Function
{
public:
    Lambda(const Node* args, const Node* exp);
    void write(std::ostream& out) const;
    const Atom* eval(Env& env) const;

private:
    const Node* exp_;
};

class Node : public Atom
{
public:
    class Iterator
    {
    friend class Node;

    public:
        Iterator(const Node* node);
        bool good() const;
        const Node* operator * () const;
        const Node* operator -> () const;
        Iterator& operator ++ ();
        Iterator operator ++ (int);

    private:
        const Node* node_;
    };

    static const Node* getNull();

    Node(const Atom* atom, const Node* next);
    void write(std::ostream& out) const;
    const Atom* car() const;
    const Node* cdr() const;
    const Atom* eval(Env& env) const;
    const Atom* evalSymbol(const Symbol* symbol, Env& env) const;
    const Atom* evalFunction(const Function* function, Env& env) const;

private:
    Node();

    const Atom* evalQuote(Iterator i, Env& env) const;
    const Atom* evalIf(Iterator i, Env& env) const;
    const Atom* evalSet(Iterator i, Env& env) const;
    const Atom* evalDefine(Iterator i, Env& env) const;
    const Atom* evalLambda(Iterator i, Env& env) const;
    const Atom* evalBegin(Iterator i, Env& env) const;

    const Atom* car_;
    const Node* cdr_;
};

#endif//ATOMS_H
