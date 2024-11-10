#ifndef AST_HH
#define AST_HH

#include <string>
#include <map>
#include <vector>

using namespace std;

namespace ast {

class SymbolTable {
    private:
        map<string, int> table;
    public:
        SymbolTable();
        int get(string);
        void set(string, int); // will work only on existing variables.
        void print();
        ~SymbolTable();
};

class Expression {
    public:
        virtual int evaluate() = 0;
        virtual ~Expression() =0;
};

class Num : public Expression {
    public:
        const int value;
        Num(int v);
        virtual int evaluate();
};

class Var : public Expression {
    public:
        const string name;
        Var(string n);
        virtual int evaluate();
};

class BinaryExpression : public Expression {
    protected:
        Expression *left;
        Expression *right;

    public:
        BinaryExpression(Expression *l, Expression *r);
        virtual ~BinaryExpression();
};

class AddExpression : public BinaryExpression {
    public:
        AddExpression(Expression *l, Expression *r);
        virtual int evaluate();
};

class SubExpression : public BinaryExpression {
    public:
        SubExpression(Expression *l, Expression *r);
        virtual int evaluate() ;
};

class MulExpression : public BinaryExpression {
    public:
        MulExpression(Expression *l, Expression *r) ;
        virtual int evaluate();
};

class DivExpression : public BinaryExpression {
    public:
        DivExpression(Expression *l, Expression *r);
        virtual int evaluate();
};
class Statement {
    public:
        virtual void run() = 0;
        virtual ~Statement() = 0;
};

class AssignmentStatement : public Statement {
private: 
    string variable;
    Expression *expression;
public:
    void run();
    AssignmentStatement(string, Expression *);
    virtual ~AssignmentStatement();
};

class SequenceStatement : public Statement {
    private:
        vector<Statement *> statements;
    public:
        void addStatement(Statement *s);
        void run();
        virtual ~SequenceStatement();
};

// Other realtional classes
class LessThanExpression : public Expression {
    Expression *left;
    Expression *right;
public:
    LessThanExpression(Expression *l, Expression *r);
    int evaluate() ;
    ~LessThanExpression();
};
class GreaterThanExpression : public Expression {
private:
    Expression *left;
    Expression *right;
public:
    GreaterThanExpression(Expression *l, Expression *r);
    int evaluate();
    ~GreaterThanExpression();
};


// Branching & looping 
class IfStatement : public Statement {
private: 
    Expression *condition;
    Statement *thenBranch;
    Statement *elseBranch;
public:
    IfStatement(Expression *, Statement *, Statement *);
    virtual void run();
    ~IfStatement();
    
};

class WhileStatement : public Statement {
private:
    Expression *condition;
    SequenceStatement *body;
public:
    WhileStatement(Expression *, SequenceStatement *);
    virtual void run();
    ~WhileStatement();
};



// Program class
class Program {
    string name;
    Statement *statement;
    SymbolTable *symbolTable;
public:
    Program(string , Statement *);
    string getName();
    SymbolTable& getSymbolTable();
    void run();
    ~Program();
};

extern Program *theProgram;

} // namespace ast

#endif // AST_HH
