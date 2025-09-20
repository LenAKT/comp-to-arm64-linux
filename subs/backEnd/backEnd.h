#include "maps/maps.h"
#include <fstream>
#include <set>
#include <iostream>

#pragma once

enum class Mode{
    intValue,
    varValue,

};

struct Reg{
    Reg(int s, int e) : start(s), end(e) {regs.resize(end-start, false);};
    std::vector<bool> regs;
    int start;
    int end;
};

class backEnd
{
    public:
    backEnd(std::shared_ptr<Scope> s): outerSCope(s) {
        std::cout << "in second " << std::endl;
        outFile.open("out.s", std::ios::trunc);
        outFile << ".text" << std::endl;
        if (!outFile)
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
    void movFunc(Mode mode, std::shared_ptr<Cvar> toVar,  int fromVar);
    int findNextFree(Reg* r);
    void Printer(std::shared_ptr<Scope> s);
    void  ArithmaFunction(std::shared_ptr<ArIr> ir, bool);
    void runner(std::shared_ptr<IrNode> i, int newAdr);
    void start(std::vector<std::shared_ptr<IrNode>>* i);
    void clearRegs(Reg* );
    void boolFunction(std::shared_ptr<getBool> b);
    void cleanUp(int i);

    void cleanPush(std::shared_ptr<Cvar> s){
        s->seen--;
        clean.push_back(s);
    }

    Reg* cReg;
    std::set<int> delteRegs;
    std::unordered_map<std::string, int> fReturns;
    std::vector<std::shared_ptr<IrNode>>* gIr;

    int labelCounter = 0;
    int uSkip = -1;

    Reg STMregs{9, 16};
    Reg FuRegs{0,8};
    Reg LTMregs{19,30};

    int spillNR;
    
    std::ofstream outFile;
    std::vector<std::shared_ptr<Cvar>> clean;
    std::shared_ptr<Scope> outerSCope;
    std::shared_ptr<Scope> scope;
};