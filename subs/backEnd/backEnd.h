#include "maps/maps.h"
#include "stack/stack.h"
#include <fstream>
#include <set>
#include <sstream>
#include <iostream>

#pragma once

enum class Mode{
    intValue,
    varValue,

};

struct ScopeSaver{
    int e;
    int u;
    bool set = false;
};

struct cleanUP{
    int position;
    ArIr savee;
};

struct Reg{
    Reg(int s, int e) : start(s), end(e) {regs.resize(end, false);};
    std::vector<bool> regs;
    int start;
    int end;
};

class backEnd
{
    public:
    backEnd(std::shared_ptr<Scope> s): outerSCope(s) {
        std::cout << "in second " << std::endl;
        
        pushFile.open("out.s", std::ios::trunc);
        pushFile << ".text" << std::endl;
        if (!pushFile)
        {
           std::cout << "couldt open file" << std::endl;
           exit(1);
        }
        scope = outerSCope;
    };

    void forceTakeR(int a);
    
    void moveToReg(std::shared_ptr<Cvar> var, int reg);
    void makeVar(std::shared_ptr<Cvar> var,  std::shared_ptr<valueNode> node, std::string );
    void makeFunction(std::string name);
    void movFunc(Mode mode, std::shared_ptr<Cvar> toVar,  std::shared_ptr<Cvar> fromVar);
    int findNextFree(Reg* r);
    void Printer(std::shared_ptr<Scope> s);
    void ArithmaFunction(std::shared_ptr<ArIr> ir, bool);
    void runner(std::shared_ptr<IrNode> i, int newAdr);
    void start(std::vector<std::shared_ptr<IrNode>>* i);
    void clearRegs(Reg* );
    void boolFunction(std::shared_ptr<getBool> b);
    int pullFromStack(std::string name);
    void cleanUp(int i);
    void setRegVlaue(std::string);
    std::string getRegValue(std::shared_ptr<Cvar>);

    void cleanPush(std::shared_ptr<Cvar> s){
        if (s->seen > s->sInfo.highestSeen)
        {
            s->sInfo.highestSeen = s->seen;
        }        
        s->seen--;
        clean.push_back(s);
    }

    Reg* cReg;

    std::shared_ptr<stackHandler> s;
    bool setStack = false;
    void makeStack(std::shared_ptr<Scope> sc){
        s = std::make_shared<stackHandler>(sc);
        setStack = true;
    };

    std::vector<std::pair<float*, std::string>> StackScores;
    std::set<int> delteRegs;
    std::unordered_map<std::string, int> fReturns;
    std::vector<std::shared_ptr<IrNode>>* gIr;

    int labelCounter = 0;

    Reg STMregs{9, 16};
    Reg FuRegs{0,8};
    Reg LTMregs{19,30};

    int spillNR = 0;
    
    std::vector<std::string> pushToStack;
    std::ofstream pushFile;
    std::stringstream outFile;
    std::vector<std::shared_ptr<Cvar>> clean;
    std::shared_ptr<Scope> outerSCope;
    std::shared_ptr<Scope> scope;
    std::vector<cleanUP> laundry;
    std::unordered_map<std::shared_ptr<Scope>, ScopeSaver> scopeSvs;
};

