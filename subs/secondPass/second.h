#include <iostream>
#include <vector>
#include "maps/maps.h"
#include "backEnd/backEnd.h"
#include <cstdint>

#pragma once

struct parseLogic
{
    bool setBool = false;
    bool collecting = false;
    bool runFunction = false;
    bool setvalue = false;
    bool returningVlaue = false;
    bool inParams = false;
};

class Second
{
    public:
    Second(std::shared_ptr<Scope>& s,const std::vector<preToken>& v, backEnd& b) : outerScope(s), preTokens(v), backend(b) {scope = outerScope;};

    void getNodeTree(Variable var,std::vector<preToken>& buffer, std::string name);
    void secondPass();

    template<typename nodeType, typename valueType>
    std::shared_ptr<valueNode> assignValue(std::shared_ptr<valueNode>& node, valueType type);

    backEnd &backend;

    std::shared_ptr<Scope> scope;
    std::shared_ptr<Scope> outerScope;
    const std::vector<preToken> preTokens;
    std::shared_ptr<NFunction> activeFunction;

    std::vector<std::shared_ptr<IrNode>> ir;
    std::vector<std::shared_ptr<IrNode>>* irSender = &ir;

    int fBreaker = 0;
    int pBreaker = 0;

    std::shared_ptr<valueNode> rDig(std::shared_ptr<operatorNode>& n);
    std::shared_ptr<valueNode> Intfolder(const std::shared_ptr<operatorNode>& node);
    std::shared_ptr<valueNode> startFold(const std::shared_ptr<operatorNode>& v, std::string, bool );
    std::shared_ptr<valueNode> stringfolder(const std::shared_ptr<operatorNode>& node);
    void startIR(std::shared_ptr<Scope> s);
    void boolhandling(std::vector<preToken>& buffer, std::string& name, Variable& activeVar,const preToken& token, int& i);
    void functionDealer(std::vector<preToken>& buffer, std::string& name, Variable& activeVar,const preToken& token, int& i);
    void makeBool(std::vector<preToken>& buffer, BoolEnum);
    std::vector<preToken> paramCollector(std::vector<preToken>& buffer, int *position);

    void delimiterHandlign(std::vector<preToken>& buffer, std::string& name, Variable& activeVar,const preToken& token, int& i);
    std::shared_ptr<functionNode> functionHandling(std::vector<preToken>& b, std::shared_ptr<NFunction>& n, std::string );
    parseLogic logic;
    int fakeNum = 0;
};






// struct procedure{
//     ~procedure();
    
//     virtual void run();

//     unique_ptr<procedure> next;
// };

// struct activate : procedure{

//     Variable var;

//     activate(Variable v) : var(v){};
//     void run() override {
//         //assesment around prefixes like const or static
//         //uintptr_t adress = getUnitPtr(maybe something);
//         //adressmap.pushback(name, adress)
//         next->run();
//     }
// };

// struct newValue : procedure{

//     Variable var;
//     valueNode value;
//     //node tree of values to be added 
//     newValue(Variable v );

// };

// struct getAdressValue
// {

// };

// struct compare : procedure{


// };

// struct loop : procedure{


// };

// struct getFunctionValue : procedure{

// };


