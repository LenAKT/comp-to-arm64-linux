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
    NONE,
    BOOLOP
};
extern const std::unordered_map<std::string, Prefix> prefixMap;

extern const std::unordered_map<std::string, TokenCategory> broadTokenMap;

extern const std::unordered_map<std::string, VarType> identifierMap;

extern const std::unordered_map<std::string, Delimiter> delimiterMap;

extern const std::unordered_map<std::string, OperatorType> operatorMap;

extern const std::unordered_map<char, std::string> arm64Ops;

extern const std::unordered_map<std::string, std::string> arm64BoolOps;

extern const std::unordered_map<std::string, std::string> arm64BoolOpsInverted;

extern const std::unordered_map<int, std::string> arm64LoadOps;

extern const std::unordered_map<int, std::string> arm64StoreOps;



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

struct Stackinfo
{
    int lastSeen = 0;
    int firstSeen = 0;
    int bigestJump = 0;
    int highestSeen = 0;
    float Score = 0;
};


struct Cvar{
    ~Cvar() {}
    std::shared_ptr<valueNode> value;
    Variable var;
    std::string name;
    Stackinfo sInfo;
    int seen = 0;
    int reg = -1;
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
    std::shared_ptr<Scope> parentS;


    bool inWhile;
    int uSkip = -1;
    int eCounter = 0;
    int emptyCounter;
};

struct valueNode
{
   virtual ~valueNode() = default;   
   virtual void print(){};
   valueType type = valueType::Error;
};

struct regOnlyNode : valueNode{
    regOnlyNode(){type = valueType::regValue;}
    int* pReg;
    int reg;
};

struct operatorNode : valueNode
{
    operatorNode(){type = valueType::Operator;}
    char c;
    bool foldCheck = false;
    std::string name;
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
    std::string ownerName;
    std::string name;
    int* reg;
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
    getBool,
    boolReturn,
    DestoyFunc
};

struct IrNode{
    virtual ~IrNode() = default;
    Instruction instruction = Instruction::Error;
    virtual std::shared_ptr<IrNode>  clone(){
        auto copy = std::make_shared<IrNode>(*this);
        return copy;
    }
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
    int reg;
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
    bool isL;
};

struct ChangeScp : IrNode{
    ChangeScp() {instruction = Instruction::ChangeSc;}
    std::shared_ptr<Scope> newScope;
};

struct boolrig{
    bool isBool = false;
    std::shared_ptr<Cvar> firstArg;
    std::shared_ptr<Cvar> secondArg;
    preToken boolArg;
    preToken nextCmpr;
    int reg;
};

enum class BoolEnum{
    IF,
    WHILE,
    ELSE,
    Else_If,
};
struct getBool : IrNode{
    getBool() {instruction = Instruction::getBool;}
    BoolEnum type;
    std::shared_ptr<Scope> bscope;
    std::vector<boolrig> subBools;
};

struct boolReturn : IrNode{
    boolReturn() {instruction = Instruction::boolReturn;}
};

struct destroyFunc : IrNode{
     destroyFunc() {instruction = Instruction::DestoyFunc;}
     std::string name; 
};