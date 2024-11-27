#include <iostream>
#include <vector>
#include "ast.hh"

using namespace std;
using namespace ast;

class Test {
protected:
    virtual Program* makeProgram() = 0;
public:
    void execute() {
        theProgram = makeProgram();
        cout << "executing " << theProgram->getName() << endl;
        theProgram->run();
        theProgram->getSymbolTable().print();
        delete theProgram;
    }
};

class Test_AssignmentStatement1 : public Test {
protected:
    virtual Program* makeProgram() {
        Statement* s = new AssignmentStatement("x", new Num(10));
        return new Program("P1", s);
    }
};

class Test_AssignmentStatement2 : public Test {
protected:
    virtual Program* makeProgram() {
        Statement* s1 = new AssignmentStatement("x", new Num(10));
        Statement* s2 = new AssignmentStatement("y", new Num(5));
        Statement* s3 = new AssignmentStatement("z", new AddExpression(new Var("x"), new Var("y")));
        SequenceStatement* slist = new SequenceStatement();
        slist->addStatement(s1);
        slist->addStatement(s2);
        slist->addStatement(s3);
        return new Program("P2", slist);
    }
};

// Branching test case implementation
// x=6;
// if(x>5) {x=10;}
// else {x=3;}

class Test_IfElseStatement : public Test {
protected:
    virtual Program* makeProgram() {
        Statement* s1 = new AssignmentStatement("x", new Num(6));
        Expression* condition = new GreaterThanExpression(new Var("x"), new Num(5));
        Statement* thenBranch = new AssignmentStatement("x", new Num(10)); 
        Statement* elseBranch = new AssignmentStatement("x", new Num(3));   
        Statement* s2 = new IfStatement(condition, thenBranch, elseBranch); 
        SequenceStatement* slist = new SequenceStatement();
        slist->addStatement(s1); 
        slist->addStatement(s2); 
        return new Program("P3", slist);
    }
};



// While looping test case
// x=1,s=0;
// while(x<11){s=s+x;x=x+1;}


class Test_WhileStatement : public Test {
protected:
    virtual Program* makeProgram() {
        Statement* s1 = new AssignmentStatement("x", new Num(1));
        Statement* s2 = new AssignmentStatement("s", new Num(0));
        Expression* condition = new LessThanExpression(new Var("x"), new Num(11)); 
        Statement* body1 = new AssignmentStatement("s", new AddExpression(new Var("s"), new Var("x"))); 
        Statement* body2 = new AssignmentStatement("x", new AddExpression(new Var("x"), new Num(1)));      
        SequenceStatement* whileBody = new SequenceStatement();
        whileBody->addStatement(body1);                                                  
        whileBody->addStatement(body2);                                                 
        Statement* s3 = new WhileStatement(condition, whileBody);
        SequenceStatement* slist = new SequenceStatement();
        slist->addStatement(s1);                                                 
        slist->addStatement(s2);                                                 
        slist->addStatement(s3);                                                
        return new Program("P4", slist);
    }
};


int main() {
    cout << "testing AST ..." << endl;
    Test_AssignmentStatement1 t1;
    Test_AssignmentStatement2 t2;
    Test_IfElseStatement t3;
    Test_WhileStatement t4;                                                           
    vector<Test*> testcases{ &t1, &t2, &t3, &t4 };
    for (auto& t : testcases) {
        t->execute();
    }
    return 0;
}


// Output:

// satyam@satyam-X542UQ:~/Desktop/assignment/mycalc$ g++ -o test_program ast.cc test_ast.cc
// satyam@satyam-X542UQ:~/Desktop/assignment/mycalc$ 
// satyam@satyam-X542UQ:~/Desktop/assignment/mycalc$ ./test_program
// testing AST ...
// executing P1
// x : 10
// executing P2
// x : 10
// y : 5
// z : 15
// executing P3
// x : 10
// executing P4
// s : 55
// x : 11