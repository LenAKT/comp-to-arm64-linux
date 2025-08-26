#include <unordered_map>
#include <string>
#include <iostream>

#pragma once

struct operatorVar;
struct NFunction;
struct Scope;
struct variablePackage;
struct Cvar;
struct valueNode;

enum class VarType {
    Bool,
    Char,
    Int,
    Float,
    String,
    Pointer,
    Void,
    NONE,
};

enum class OperatorType {
    Add,         // +
    Subtract,    // -
    Multiply,    // *
    Divide,      // /
    Modulus,     // %
    Assign,      // =
};


enum class Delimiter {
    ParenthesisOpen,    // "("
    ParenthesisClose,   // ")"
    BraceOpen,          // "{"
    BraceClose,         // "}"
    Comma,              // ","
    Semicolon,          // ";"
    NONE
};

enum class Prefix {
    Static,
    Const,
    Signed,
    Unsigned,
    NONE
};


enum class TokenCategory {
    KEYWORD,
    OPERATOR,
    DELIMITER,
    IDENTIFIER,
    VARIABLE,
    NUMBER,
    PREFIX,
    STRING,
    NONE
};
extern const std::unordered_map<std::string, Prefix> prefixMap;

extern const std::unordered_map<std::string, TokenCategory> broadTokenMap;

extern const std::unordered_map<std::string, VarType> identifierMap;

extern const std::unordered_map<std::string, Delimiter> delimiterMap;

extern const std::unordered_map<std::string, OperatorType> operatorMap;

extern const std::unordered_map<char, std::string> arm64Ops;



struct preToken
{
    std::string stringValue;
    TokenCategory cat;
};

struct Variable
{
    std::vector<Prefix> prefixes;
    VarType type;
};


struct Cvar{
    std::shared_ptr<valueNode> value;
    Variable var;
    uintptr_t adress;
    int reg = 99;
    bool is4byte = true;
};

enum class valueType{
    String,
    Intager,
    Func,
    Varible,
    Operator,
    regValue,
    Error
};

struct NFunction{
    Prefix prefix;
    VarType returnType;
    std::shared_ptr<Scope> parentScope;
    std::shared_ptr<Scope> scope;
    std::vector<preToken> fContainer;
    std::string name;
    std::unordered_map<std::string, Variable> paramMap;
    std::vector<std::string> paramOrder;
};

struct Scope{
    std::unordered_map<std::string, std::shared_ptr<NFunction>> functionMap;
    std::unordered_map<std::string, Variable> variableMap;
    std::vector<std::string> insertioOrder;
    std::unordered_map<std::string, std::shared_ptr<Cvar>> Cvars;
};

struct valueNode
{
   virtual ~valueNode() = default;   
   virtual void print(){};
   valueType type = valueType::Error;
};

struct regOnlyNode : valueNode{
    regOnlyNode(){type = valueType::regValue;}
    int reg;
};

struct operatorNode : valueNode
{
    operatorNode(){type = valueType::Operator;}
    char c;
    bool foldCheck = false;
    void print() override {
        if(l) l->print();
        std::cout << c;
        if(r) r->print();
    }
    std::shared_ptr<valueNode> l;
    std::shared_ptr<valueNode> r;
};
struct intNode : valueNode
{
    intNode(){type = valueType::Intager;}
    void print() override {
        std::cout << value;
    }
    int value;
};
struct stringNode : valueNode
{
    stringNode(){type = valueType::String;}
    void print() override{
        std::cout << value;
    }
    std::string value;
};
struct functionNode : valueNode
{
    functionNode() {type = valueType::Func;}
    std::vector<preToken> paramVars;
    std::string name;
    void print() override{
        std::cout << name;
    }
};
struct variableNode : valueNode
{
    variableNode(std::string n) : name(n) {type = valueType::Varible;}
    std::string name;
    void print() override{
        std::cout << name;
    }
};

enum class Instruction{
    Error,
    DecVar,
    DecFunc,
    RetValue,
    Arithma,
    ChangeSc,
    getFuncVal,
};

struct IrNode{
    virtual ~IrNode() = default;
    Instruction instruction = Instruction::Error;
};

struct DeclareVar : IrNode{
    DeclareVar(){instruction = Instruction::DecVar;}
    std::shared_ptr<Cvar> toDeclare;
    std::shared_ptr<valueNode> value;
    std::string name;
};

struct DeclareFunction : IrNode
{
    DeclareFunction(){instruction = Instruction::DecFunc;}
    std::string name;
};

struct getFuncVal : IrNode
{
    getFuncVal(){ instruction = Instruction::getFuncVal;}
    std::string name;
    std::vector<preToken> params;
    std::string callerName;
};

struct ReturnValue : IrNode{
    ReturnValue(){ instruction = Instruction::RetValue;}
};

struct ArIr : IrNode{
    ArIr(){instruction = Instruction::Arithma;}
    char c;
    std::string caller;
    std::shared_ptr<valueNode> parent;
    std::shared_ptr<valueNode> l;
    std::shared_ptr<valueNode> r;
};

struct ChangeScp : IrNode{
    ChangeScp() {instruction = Instruction::ChangeSc;}
    std::shared_ptr<Scope> newScope;
};