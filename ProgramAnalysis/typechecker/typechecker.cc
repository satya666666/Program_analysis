#include <iostream>
#include <algorithm>
#include "ast.hh"
#include "typechecker.hh"

using namespace std;
using namespace ast;

namespace typechecker {
// function added to return INT, BOOL, VOID, NIL
       string getType(Type T){
    switch (T)
    {
    case INT:{ return "INT";
        break;}
    case BOOL: {return "BOOL";
    break;
    }
    case VOID:{
        return "VOID";
        break;
    }
    case NIL:{
        return "NIL";
        break;
    } 
    
    default:
        return "UnExpected Type";
        break;
    }
}

set<Type> Typechecker::setUnion(set<Type>& set1, set<Type>& set2) {
    vector<Type> unionVector(set1.size() + set2.size());

    auto it = set_union(set1.begin(), set1.end(), set2.begin(),
                    set2.end(), unionVector.begin());

    // Resize the vector to remove the unused elements
    unionVector.resize(it - unionVector.begin());
    set<Type> unionSet(unionVector.begin(), unionVector.end());
    return unionSet;
}

set<Type> Typechecker::setMinus(set<Type>& set1, set<Type>& set2) {
    set<Type> newSet;
    for(auto& el : set1) {
        if(set2.find(el) == set2.end()) {
            newSet.insert(el);
        }
    }
    return newSet;
}

template <typename T> Env<T>::Env(Env<T> *p) : parent(p) {}

template <typename T> Env<T> *Env<T>::getParent() {
    return parent;
}

template <typename T> void Env<T>::print() {
}

template <typename T> T Env<T>::get(string vname) {

    if(table.find(vname) != table.end()) {
        return table[vname];
    }
    if(parent != NULL) {
        return parent->get(vname);
    }
    throw ("Variable " + vname + " not found.");
}

template <typename T> void Env<T>::addMapping(string name, T value) {
    if(table.find(name) == table.end()) {
        table[name] = value;
    }
    else {
        string m = "Env::addMapping : repeat declaration for name " + name + ".";
        throw m;
    }
}

template <typename T> Env<T>::~Env() {}

ValueEnv::ValueEnv(ValueEnv *p) : Env(p) {}

void ValueEnv::print() {
    for(auto &d : table) {
        
        cout << d.first << " : " << getType(d.second )<< endl;
    }
}

vector<Type> FunctionTypeSig::getParameterTypes() {
    return parameterTypes;
}

Type FunctionTypeSig::getReturnType() {
    return returnType;
}

FunctionTypeSig::FunctionTypeSig(vector<Type>& partypes, Type rettype) {

    for(auto& ptype : partypes) {
        parameterTypes.push_back(ptype);
    }
    returnType = rettype;
}

FunctionTypeSig::FunctionTypeSig() {}

FunctionTypeSig& FunctionTypeSig::operator=(FunctionTypeSig& fsig) {
    for(auto& ptype : fsig.parameterTypes) {
        parameterTypes.push_back(ptype);
    }
    returnType = fsig.returnType;
    return *this;
}

string FunctionTypeSig::toString() {
    string sig = "(";
    for(auto& partype : parameterTypes) {
        sig += to_string(partype) + ", ";
    }
    sig += ") ===> ";
    sig += getType(returnType);
    return sig;
}

FunctionEnv::FunctionEnv(FunctionEnv *p) : Env(p) {}
FunctionEnv::FunctionEnv() : Env(NULL) {}

void FunctionEnv::print() {
    for(auto &d : table) {
        
        cout << d.first << " : " << d.second.toString() << endl;
    }
}

Typechecker::Typechecker() {
    valueEnv = new ValueEnv(NULL);
}

Typechecker::~Typechecker() {
    delete valueEnv;
}

ValueEnv& Typechecker::getValueEnv() {
    return *valueEnv;
}

FunctionEnv& Typechecker::getFunctionEnv() {
    return functionEnv;
}

void Typechecker::enterScope() {
    ValueEnv *newScope = new ValueEnv(valueEnv);
    valueEnv = newScope;
}

void Typechecker::exitScope() {
    ValueEnv *newScope = dynamic_cast<ValueEnv *>(valueEnv->getParent());
    delete valueEnv;
    valueEnv = newScope;
}

Type Typechecker::typecheckVar(Var& v) {
    return valueEnv->get(v.name);
}

Type Typechecker::typecheckExpression(Expression& e) {
    switch(e.exprtype) {
        case EMPTY:
            return typecheckEmpty(dynamic_cast<Empty&>(e));
        case VAR:
            return typecheckVar(dynamic_cast<Var&>(e));
        case NUM:
            return typecheckNum(dynamic_cast<Num&>(e));
        case BINARY:
            return typecheckBinaryExpression(dynamic_cast<BinaryExpression&>(e));
        case FUNCTIONCALL:
            return typecheckFunctionCall(dynamic_cast<FunctionCall&>(e));
        default:
        string m = "Typechecker::typecheckExpression : Unknown expression type!" + to_string(e.exprtype);
            throw m;
    }
}

Type Typechecker::typecheckEmpty(Empty& e) {
    return VOID;
}

Type Typechecker::typecheckNum(Num& n) {
    return INT;
}

Type Typechecker::typecheckBinaryExpression(BinaryExpression& e) {
    if((typecheckExpression(e.getLeft()) == INT) &&
            (typecheckExpression(e.getRight()) == INT)) {
        return INT;
    }
    
    throw "Typechecker::typecheckBinaryExpression : Incorrect operand type!";
}

// TODO
Type Typechecker::typecheckFunctionCall(FunctionCall& funccall) {
 try {
        // Retrieve the function signature from the environment
        FunctionTypeSig funcSig = functionEnv.get(funccall.getName());
        
        // Check if the number of arguments matches the number of parameters
        vector<Type> paramTypes = funcSig.getParameterTypes();
        vector<Expression *> args = funccall.getArguments();
        
        if (paramTypes.size() != args.size()) {
            throw "Typechecker::typecheckFunctionCall : Argument count mismatch in function call to " + funccall.getName();
        }

        // Check if each argument matches the corresponding parameter type
        for (size_t i = 0; i < args.size(); ++i) {
            Type argType = typecheckExpression(*args[i]);
            if (argType != paramTypes[i]) {
                throw "Typechecker::typecheckFunctionCall : Argument type mismatch in function call to " + funccall.getName();
            }
        }

        // Return the return type of the function
        return funcSig.getReturnType();
    } catch (const string& e) {
        throw "Typechecker::typecheckFunctionCall : Undefined function " + funccall.getName();
    }
}

set<Type> Typechecker::typecheckStatement(Statement& stmt) {
    switch(stmt.stmttype) {
        case SKIP:
            return typecheckSkip(dynamic_cast<SkipStatement&>(stmt));
        case ASSIGN:
            return typecheckAssignment(dynamic_cast<AssignmentStatement&>(stmt));
        case BRANCH:
            return typecheckBranch(dynamic_cast<BranchStatement&>(stmt));
        case LOOP:
            return typecheckLoop(dynamic_cast<LoopStatement&>(stmt));
        case SEQUENCE:
            return typecheckSequence(dynamic_cast<SequenceStatement&>(stmt));
        case BLOCK:
            return typecheckBlock(dynamic_cast<BlockStatement&>(stmt));
        case RETURN:
            return typecheckReturn(dynamic_cast<ReturnStatement&>(stmt));
        default:
            throw "Typechecker::typecheckStatement : Unknown statement type!";
    }
}

set<Type> Typechecker::typecheckSkip(SkipStatement& stmt) {
    return {NIL};
}

set<Type> Typechecker::typecheckAssignment(AssignmentStatement& stmt) {
    if(valueEnv->get(stmt.getVariable()) ==
            typecheckExpression(*(stmt.getExpression()))) {
        return {NIL};
    }
    throw "Typechecker::typecheckAssignmentStatement : type mismatch!";
}

set<Type> Typechecker::typecheckBranch(BranchStatement& branch) {
	 
      BinaryExpression& ltExpr = dynamic_cast<BinaryExpression&>(branch.getCondition());
        Type leftType = typecheckExpression(ltExpr.getLeft());  // Typecheck the left operand
        Type rightType = typecheckExpression(ltExpr.getRight());  // Typecheck the right operand
        if (leftType !=rightType) {    
            throw "Typechecker::typecheckExpression: Operands must be of same Type!";
        }
    
    set<Type> thenTypes = typecheckStatement(branch.getThenStatement());
    set<Type> elseTypes = typecheckStatement(branch.getElseStatement());

    return setUnion(thenTypes, elseTypes);
}

// TODO
set<Type> Typechecker::typecheckLoop(LoopStatement& loop) {
	 if (typecheckExpression(loop.getCondition()) != BOOL) {
        throw "Typechecker::typecheckLoop: Condition is not of type BOOL!";
    }

    // Typecheck the body of the loop
    set<Type> bodyTypes = typecheckStatement(loop.getBody());

    // Return the possible types from the body
    return bodyTypes;
}

// TODO
set<Type> Typechecker::typecheckSequence(SequenceStatement& stmt) {
	set<Type> resultTypes;
    // Typecheck each statement in the sequence
    for (auto& s : stmt.getStatements()) {
        set<Type> stmtTypes = typecheckStatement(*s);
        resultTypes = setUnion(resultTypes, stmtTypes);
    }
    return resultTypes;
}

// TODO
set<Type> Typechecker::typecheckBlock(BlockStatement& stmt) {
	// Enter a new scope for the block
    enterScope();

    // Typecheck each declaration in the block and add it to the environment
    for (auto& decl : stmt.getDeclarations()) {
     
        valueEnv->addMapping(decl->getVariable(), decl->getType());
    }
    // Typecheck the block's statement (which could be a sequence or any other statement)
    set<Type> blockTypes = typecheckStatement(stmt.getStatement());
    
    // Exit the block's scope
    exitScope();
    
    return blockTypes;
}

set<Type> Typechecker::typecheckReturn(ReturnStatement& stmt) {
    set<Type> returnTypes = {typecheckExpression(stmt.getExpression())};
    return returnTypes;
}

// TODO
void Typechecker::typecheckFunctionDef(FunctionDefinition& fdef) {
     // for type checking the function def
    // we need to check the return type and parameters 
    // 1. Get the return type and parameters of the functions
    Type returnType = fdef.getReturnType();
    vector<Declaration *> params = fdef.getParameters();
    
    // Collect the parameter types
    vector<Type> paramTypes;
    for (auto& param : params) {
        paramTypes.push_back(param->getType());
    }
   
    FunctionTypeSig funcSig(paramTypes, returnType);
    functionEnv.addMapping(fdef.getName(), funcSig);
    enterScope();
    for (auto& param : params) {
        valueEnv->addMapping(param->getVariable(), param->getType());
    }

    BlockStatement& body = fdef.getBody();
    
    set<Type> returnTypes = typecheckBlock(body);
    if(returnTypes.size()>2){
        throw "Typechecker::typecheckFunctionDef : More than 2 Return type in function " + fdef.getName();
    }
    if (returnTypes.find(returnType) == returnTypes.end()) {
        throw "Typechecker::typecheckFunctionDef : Return type mismatch in function " + fdef.getName();
    }
   
    // 6. Exit the function's local scope
    exitScope();
}

void Typechecker::typecheckProgram(Program& program) {
    for(auto& d : program.getDeclarations()) {
        valueEnv->addMapping(d->getVariable(), d->getType());
    }
    for(auto& fdef : program.getFunctionDefs()) {
        typecheckFunctionDef(*fdef);
    }
    typecheckStatement(program.getStatement());
}

} // namespace typechecker
