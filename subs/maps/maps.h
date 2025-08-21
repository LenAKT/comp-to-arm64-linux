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
};


struct NFunction{
    Prefix prefix;
    VarType returnType;
    std::shared_ptr<valueNode> returnValue;
    std::shared_ptr<Scope> parentScope;
    std::shared_ptr<Scope> scope;
    std::vector<preToken> fContainer;
    std::string name;
    std::unordered_map<std::string, Variable> paramMap;

};

struct Scope{
    std::unordered_map<std::string, std::shared_ptr<NFunction>> functionMap;
    std::unordered_map<std::string, Variable> variableMap;
    std::unordered_map<std::string, std::shared_ptr<Cvar>> Cvars;
};

enum class valueType{
    String,
    Intager,
    Func,
    Varible,
    Operator
};

struct valueNode
{
   virtual ~valueNode() = default;   
   virtual void print(){};
   valueType type;
};

struct operatorNode : valueNode
{
    operatorNode(){type = valueType::Operator;}
    char c;
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
    functionNode(std::string n) : name(n) {type = valueType::Func;}
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
