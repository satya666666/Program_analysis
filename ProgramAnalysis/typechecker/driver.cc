#include <iostream>
#include "ast.hh"
#include "typechecker.hh"

using namespace std;
using namespace ast;
using namespace typechecker;
class Test {

    protected:
        virtual Program *makeProgram() = 0;
    public:
        void execute() {
            theProgram = makeProgram();
            cout << "------------------------typechecking " << theProgram->getName() << "--------------------" << endl;
            Typechecker typechecker;
            typechecker.typecheckProgram(*theProgram);
            cout << "Value environment:" << endl;
            typechecker.getValueEnv().print();
            cout << "Function environment:" << endl;
            typechecker.getFunctionEnv().print();
            cout << "*-----------------------------------------------------------" << endl;
            delete theProgram;
        }
};
/*
Input:
*****************************
    int x
    x := 10
*****************************
*/
class T1 : public Test {
    protected:
        virtual Program *makeProgram() {
            Declaration *d1 = new Declaration("x", INT);
            vector<Declaration *> declarations = { d1 };
            Statement *s = new AssignmentStatement("x", new Num(10));
            vector<FunctionDefinition *> fdefs;
            return new Program("P1", declarations, fdefs, s);
        }
};

/*
Input:
*****************************
    int x
    int y
    x := 10
    y := 1
*****************************
*/
class T2 : public Test {
    protected:
        virtual Program *makeProgram() {
            Declaration *d1 = new Declaration("x", INT);
            Declaration *d2 = new Declaration("y", INT);
            vector<Declaration *> declarations = { d1, d2 };
            Statement *s1 = new AssignmentStatement("x", new Num(10));
            Statement *s2 = new AssignmentStatement("y", new Num(1));
            SequenceStatement *seq1 = new SequenceStatement();
            seq1->addStatement(s1);
            seq1->addStatement(s2);
            vector<FunctionDefinition *> fdefs;
            return new Program("P2", declarations, fdefs, seq1);
        }
};

/*
Input:
*****************************
  int x;
  int add(int x, int y) { return x + y }
  x := add(1, 2)
*****************************
*/
class T3 : public Test {
    protected:
        virtual Program *makeProgram() {
        
            Declaration *d1 = new Declaration("x", INT); // int x
            vector<Declaration *> declarations{d1};

            Declaration *p1 = new Declaration("x", INT); // parameter 1
            Declaration *p2 = new Declaration("y", INT); // parameter 2
            vector<Declaration *> params = {p1, p2}; // parameter list
            Var *v1 = new Var("x");
            Var *v2 = new Var("y");
            AddExpression *e1 = new AddExpression(v1, v2); // x + y
            ReturnStatement *s1 = new ReturnStatement(e1); // return x + y
            BlockStatement *b1 = new BlockStatement(s1); // { return x + y }
            FunctionDefinition *fd1 = 
                new FunctionDefinition("add", INT, params, b1);
                    // int add(int x, int y) { return x + y }
            vector<FunctionDefinition *> fdefs{fd1};
            Num *a1 = new Num(1); // 1
            Num *a2 = new Num(2); // 2
            vector<Expression *> args{a1, a2}; // (1, 2)
            FunctionCall *fc1 = new FunctionCall("add", args); // add(1, 2)
            AssignmentStatement *s2 = 
                new AssignmentStatement("x", fc1); // x := add(1, 2)

            return new Program("P3", declarations, fdefs, s2);
        }
};

/*
Input:
*****************************
  int x;
  void add() { return }
  x := add()
****************************
*/
class T4 : public Test {
    protected:
        virtual Program *makeProgram() {
        
            Declaration *d1 = new Declaration("x", INT); // int x
            vector<Declaration *> declarations{d1};

            vector<Declaration *> params = {}; // parameter list
            Empty *e1 = new Empty(); // empty
            ReturnStatement *s1 = new ReturnStatement(e1); // return empty
            BlockStatement *b1 = new BlockStatement(s1); // { return empty }
            FunctionDefinition *fd1 = 
                new FunctionDefinition("f", INT, params, b1);
                    // int add() { return empty }
            vector<FunctionDefinition *> fdefs{fd1};
            vector<Expression *> args{}; // ()
            FunctionCall *fc1 = new FunctionCall("add", args); // add()
            AssignmentStatement *s2 = 
                new AssignmentStatement("x", fc1); // x := add()

            return new Program("P4", declarations, fdefs, s2);
        }
};

/*
Input:
*****************************
  int x;
  if (x == 10) {
    x := 20;
  } else {
    x := 30;
  }
*****************************
*/
class T5 : public Test {
    protected:
        virtual Program *makeProgram() {
            // Declaration
            Declaration *d1 = new Declaration("x", INT);

            // Condition: x == 10
              
            BinaryExpression *cond = new BinaryExpression( new Var("x"), new Num(10) );

            // Then block: x := 20
            Num *twenty = new Num(20);
            AssignmentStatement *thenStmt = new AssignmentStatement("x", twenty);

            // Else block: x := 30
            Num *thirty = new Num(30);
            AssignmentStatement *elseStmt = new AssignmentStatement("x", thirty);

            // Branch Statement
            BranchStatement *branchStmt = new BranchStatement(cond, thenStmt, elseStmt);

            vector<Declaration *> declarations = { d1 };
            vector<FunctionDefinition *> fdefs;
            return new Program("P5_Branch_Test", declarations, fdefs, branchStmt);
        }
};
/*
Input:
*****************************
  int x;
  while (x < 10) {
    x := x + 1;
  }
*****************************
*/
class T6 : public Test {
    protected:
        virtual Program *makeProgram() {
            // Declaration
            Declaration *d1 = new Declaration("x", INT);

            // Condition: x < 10
            Var *x = new Var("x");
            BinaryExpression *cond = new SubExpression(x, new Num(10));

            // Body: x := x + 1
            AddExpression *increment = new AddExpression(x, new Num(1));
            AssignmentStatement *bodyStmt = new AssignmentStatement("x", increment);

            // Loop Statement
            LoopStatement *loopStmt = new LoopStatement(cond, bodyStmt);

            vector<Declaration *> declarations = { d1 };
            vector<FunctionDefinition *> fdefs;
            return new Program("P6_Loop_Test", declarations, fdefs, loopStmt);
        }
};
/*
Input:
*****************************
  int x;
  x := 1;
  x := x + 1;
*****************************
*/
class T7 : public Test {
    protected:
        virtual Program *makeProgram() {
            // Declaration
            Declaration *d1 = new Declaration("x", INT);

            // First Assignment: x := 1
            AssignmentStatement *stmt1 = new AssignmentStatement("x", new Num(1));

            // Second Assignment: x := x + 1
            Var *x = new Var("x");
            AddExpression *increment = new AddExpression(x, new Num(1));
            AssignmentStatement *stmt2 = new AssignmentStatement("x", increment);

            // Sequence Statement
            SequenceStatement *seqStmt = new SequenceStatement();
            seqStmt->addStatement(stmt1);
            seqStmt->addStatement(stmt2);

            vector<Declaration *> declarations = { d1 };
            vector<FunctionDefinition *> fdefs;
            return new Program("P7_Sequence_Test", declarations, fdefs, seqStmt);
        }
};
/*
Input:
*****************************
  int x;
  {
    int y;
    y := 10;
    x := y + 1;
  }
*****************************
*/
class T8 : public Test {
protected:
    virtual Program *makeProgram() {
        // Declaration for x
        Declaration *d1 = new Declaration("x", INT);

        // Declaration for y inside the block
        Declaration *d2 = new Declaration("y", INT);

        // Assignment: y := 10
        AssignmentStatement *assignY = new AssignmentStatement("y", new Num(10));

        // Assignment: x := y + 1
        Var *y = new Var("y");
        AddExpression *expr = new AddExpression(y, new Num(1));
        AssignmentStatement *assignX = new AssignmentStatement("x", expr);

        // Create a sequence of statements for the block
        SequenceStatement *blockSeq = new SequenceStatement();
        blockSeq->addStatement(assignY);
        blockSeq->addStatement(assignX);

        // Block Statement with the local declaration and sequence of statements
        BlockStatement *blockStmt = new BlockStatement(blockSeq);

        vector<Declaration *> declarations = {d1};
        vector<FunctionDefinition *> fdefs;
        return new Program("P8_Block_Test", declarations, fdefs, blockStmt);
    }
};


/*
Input:
*****************************
  int x;
  if (x > 10) {
    x := 20;
  } else {
    x := 30;
  }
*****************************
*/
class T9 : public Test {
protected:
    virtual Program *makeProgram() {
        // Declaration for x
        Declaration *d1 = new Declaration("x", INT);

        // Condition: x > 10 (this should evaluate to BOOL)
        Num *x = new Num(11);
        Num *ten = new Num(10);
        Var *v = new Var("d");
        BinaryExpression *cond = new BinaryExpression(x,new Num(10));  // x > 10
        // Then block: x := 20
        AssignmentStatement *thenStmt = new AssignmentStatement("x", new Num(20));

        // Else block: x := 30
        AssignmentStatement *elseStmt = new AssignmentStatement("x", new Num(30));

        // If-else statement
        BranchStatement *branchStmt = new BranchStatement(cond, thenStmt, elseStmt);

        vector<Declaration *> declarations = {d1};
        vector<FunctionDefinition *> fdefs;
        return new Program("T9", declarations, fdefs, branchStmt);
    }
};
/*
Input:
*****************************
    char x
    x := 10
*****************************
*/
class T10 : public Test {
    protected:
        virtual Program *makeProgram() {
            Declaration *d1 = new Declaration("x", CHAR);
            vector<Declaration *> declarations = { d1 };
            Statement *s = new AssignmentStatement("x", new Num(1));
            vector<FunctionDefinition *> fdefs;
            string name ="---------P10 -- Integer in CHAR------------";
            return new Program(name, declarations, fdefs, s);
        }
};
/*
Input:
*****************************
    char x
    char y
    x := "1"
    y := "2"
*****************************
*/
class T11 : public Test {
    protected:
        virtual Program *makeProgram() {
            Declaration *d1 = new Declaration("x", CHAR);
            Declaration *d2 = new Declaration("y", CHAR);
            vector<Declaration *> declarations = { d1, d2 };
            Statement *s1 = new AssignmentStatement("x", new Num(1));
            Statement *s2 = new AssignmentStatement("y", new Num(2));
            SequenceStatement *seq1 = new SequenceStatement();
            seq1->addStatement(s1);
            seq1->addStatement(s2);
            vector<FunctionDefinition *> fdefs;
            return new Program("P11 ", declarations, fdefs, seq1);
        }
};
/*
Input:
*****************************
    int x
    char y
    x := '1'
    y := 1
*****************************
*/
// TODO
class T12 : public Test {
    protected:
        virtual Program *makeProgram() {
            Declaration *d1 = new Declaration("x", INT);
            Declaration *d2 = new Declaration("y", CHAR);
            vector<Declaration *> declarations = { d1, d2 };
            Statement *s1 = new AssignmentStatement("x", new Num(1));
            Statement *s2 = new AssignmentStatement("y", new Num(2));
            SequenceStatement *seq1 = new SequenceStatement();
            seq1->addStatement(s1);
            seq1->addStatement(s2);
            vector<FunctionDefinition *> fdefs;
            return new Program("P12", declarations, fdefs, seq1);
        }
};
/*
Input:
*****************************
  int x;
  int add(int x, int y) { return x + y }
  x := add(1, 2)
*****************************
*/
class T13 : public Test {
    protected:
        virtual Program *makeProgram() {
        
            Declaration *d1 = new Declaration("x", INT); // int x
            vector<Declaration *> declarations{d1};

            Declaration *p1 = new Declaration("x", INT); // parameter 1
            Declaration *p2 = new Declaration("y", INT); // parameter 2
            vector<Declaration *> params = {p1, p2}; // parameter list
            Var *v1 = new Var("x");
            Var *v2 = new Var("y");
            SubExpression *e1 = new SubExpression(v1, v2); // x + y
            ReturnStatement *s1 = new ReturnStatement(e1); // return x + y
            BlockStatement *b1 = new BlockStatement(s1); // { return x + y }
            FunctionDefinition *fd1 = 
                new FunctionDefinition("Sub", INT, params, b1);
                    // int add(int x, int y) { return x + y }
            vector<FunctionDefinition *> fdefs{fd1};
            Num *a1 = new Num(1); // 1
            Num *a2 = new Num(2); // 2
            vector<Expression *> args{a1, a2}; // (1, 2)
            FunctionCall *fc1 = new FunctionCall("Sub", args); // add(1, 2)
            AssignmentStatement *s2 = 
                new AssignmentStatement("x", fc1); // x := add(1, 2)

            return new Program("P13", declarations, fdefs, s2);
        }
};
/*
Input:
*****************************
  int x;
  void f() { return }
  x := f()
*****************************
*/
class T14 : public Test {
    protected:
        virtual Program *makeProgram() {
        
            Declaration *d1 = new Declaration("x", INT); // int x
            vector<Declaration *> declarations{d1};

            vector<Declaration *> params = {}; // parameter list
            Empty *e1 = new Empty(); // empty
            ReturnStatement *s1 = new ReturnStatement(e1); // return empty
            BlockStatement *b1 = new BlockStatement(s1); // { return empty }
            FunctionDefinition *fd1 = 
                new FunctionDefinition("add", INT, params, b1);
                    // int add(int x, int y) { return empty }
            vector<FunctionDefinition *> fdefs{fd1};
            vector<Expression *> args{}; // ()
            FunctionCall *fc1 = new FunctionCall("add", args); // add()
            AssignmentStatement *s2 = 
                new AssignmentStatement("x", fc1); // x := add()

            return new Program("P14", declarations, fdefs, s2);
        }
};

/*
Input:
*****************************
  int x=10;
  if (x == 10) {
    x := 20;
  } else {
    x := 30;
  }
*****************************
*/
class T15 : public Test {
protected:
    virtual Program *makeProgram() {
        // Declaration for x
        Declaration *d1 = new Declaration("x", INT);
         
        // Condition: x = 10 (this should evaluate to BOOL)
        AssignmentStatement *valx= new AssignmentStatement("x", new Num(10));

        EqualExpression *cond = new EqualExpression(valx->getExpression(),new Num(10));  // x == 10
        // Then block: x := 20
        AssignmentStatement *thenStmt = new AssignmentStatement("x", new Num(20));

        // Else block: x := 30
        AssignmentStatement *elseStmt = new AssignmentStatement("x", new Num(30));

        // If-else statement
        BranchStatement *branchStmt = new BranchStatement(cond, thenStmt, elseStmt);

        vector<Declaration *> declarations = {d1};
        vector<FunctionDefinition *> fdefs;
        return new Program("T15 Equal Program", declarations, fdefs, branchStmt);
    }
};
/*
Input:
*****************************
  int x=10;
  if (x > 10) {
    x := 20;
  } else {
    x := 30;
  }
*****************************
*/
class T16 : public Test {
protected:
    virtual Program *makeProgram() {
        // Declaration for x
        Declaration *d1 = new Declaration("x", INT);
         
        // Condition: x > 10 (this should evaluate to BOOL)
        AssignmentStatement *valx= new AssignmentStatement("x", new Num(10));

        MoreThenExpression *cond = new MoreThenExpression(valx->getExpression(),new Num(10));  // x == 10
        // Then block: x := 20
        AssignmentStatement *thenStmt = new AssignmentStatement("x", new Num(20));

        // Else block: x := 30
        AssignmentStatement *elseStmt = new AssignmentStatement("x", new Num(30));

        // If-else statement
        BranchStatement *branchStmt = new BranchStatement(cond, thenStmt, elseStmt);

        vector<Declaration *> declarations = {d1};
        vector<FunctionDefinition *> fdefs;
        return new Program("T16 More Than", declarations, fdefs, branchStmt);
    }
};
/*
Input:
*****************************
  int x=10;
  if (x < 10) {
    x := 20;
  } else {
    x := 30;
  }
*****************************
*/
class T17 : public Test {
protected:
    virtual Program *makeProgram() {
        // Declaration for x
        Declaration *d1 = new Declaration("x", INT);
         
        // Condition: x < 10 (this should evaluate to BOOL)
        AssignmentStatement *valx= new AssignmentStatement("x", new Num(10));

        LessThenExpression *cond = new LessThenExpression(valx->getExpression(),new Num(10));  // x == 10
        // Then block: x := 20
        AssignmentStatement *thenStmt = new AssignmentStatement("x", new Num(20));

        // Else block: x := 30
        AssignmentStatement *elseStmt = new AssignmentStatement("x", new Num(30));

        // If-else statement
        BranchStatement *branchStmt = new BranchStatement(cond, thenStmt, elseStmt);

        vector<Declaration *> declarations = {d1};
        vector<FunctionDefinition *> fdefs;
        return new Program("T17 Less Than", declarations, fdefs, branchStmt);
    }
};
int main() {
    vector<Test *> testcases = {
        
        new T1(),
        new T2(),
        new T3(),
        new T4(),
        new T5(),
        new T6(),
        new T7(),
        new T8(),
        new T9(),
        new T10(),
        new T11(),
        new T12(),
        new T13(),
        new T14(),
        new T15(),
        new T16(),
        new T17()
        
    };
        for(auto& t : testcases) {
            cout<<"\n\n";

    try {
            t->execute();
            delete t;
    }
    catch(const string& m) {
        cout << "Typechecker exception : " << m << endl;
    }
    catch(const char* m) {
        cout << "Typechecker exception : " << m << endl;
    }
    // catch(...) {
    //     cout << "Typechecker exception : " << endl;
    // }
    }
    return 0;
}

// Output: 

// satyam@satyam-X542UQ:~/Desktop/typechecker$ make
// g++ -g -c driver.cc
// g++ -g -o driver driver.o typechecker.o ast.o
// satyam@satyam-X542UQ:~/Desktop/typechecker$ ./driver


// ------------------------typechecking P1--------------------
// Value environment:
// x : INT
// Function environment:
// *-----------------------------------------------------------


// ------------------------typechecking P2--------------------
// Value environment:
// x : INT
// y : INT
// Function environment:
// *-----------------------------------------------------------


// ------------------------typechecking P3--------------------
// Value environment:
// x : INT
// Function environment:
// add : (0, 0, ) ===> INT
// *-----------------------------------------------------------


// ------------------------typechecking P4--------------------
// Typechecker exception : Typechecker::typecheckFunctionDef : Return type mismatch in function f


// ------------------------typechecking P5_Branch_Test--------------------
// Value environment:
// x : INT
// Function environment:
// *-----------------------------------------------------------


// ------------------------typechecking P6_Loop_Test--------------------
// Typechecker exception : Typechecker::typecheckLoop: Condition is not of type BOOL!


// ------------------------typechecking P7_Sequence_Test--------------------
// Value environment:
// x : INT
// Function environment:
// *-----------------------------------------------------------


// ------------------------typechecking P8_Block_Test--------------------
// Typechecker exception : Variable y not found.


// ------------------------typechecking T9--------------------
// Value environment:
// x : INT
// Function environment:
// *-----------------------------------------------------------


// ------------------------typechecking ---------P10 -- Integer in CHAR--------------------------------
// Typechecker exception : Typechecker::typecheckAssignmentStatement : type mismatch!


// ------------------------typechecking P11 --------------------
// Typechecker exception : Typechecker::typecheckAssignmentStatement : type mismatch!


// ------------------------typechecking P12--------------------
// Typechecker exception : Typechecker::typecheckAssignmentStatement : type mismatch!


// ------------------------typechecking P13--------------------
// Value environment:
// x : INT
// Function environment:
// Sub : (0, 0, ) ===> INT
// *-----------------------------------------------------------


// ------------------------typechecking P14--------------------
// Typechecker exception : Typechecker::typecheckFunctionDef : Return type mismatch in function add


// ------------------------typechecking T15 Equal Program--------------------
// Value environment:
// x : INT
// Function environment:
// *-----------------------------------------------------------


// ------------------------typechecking T16 More Than--------------------
// Value environment:
// x : INT
// Function environment:
// *-----------------------------------------------------------


// ------------------------typechecking T17 Less Than--------------------
// Value environment:
// x : INT
// Function environment:
// *-----------------------------------------------------------