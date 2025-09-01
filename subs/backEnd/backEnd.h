#include "maps/maps.h"
#include <fstream>
#include <set>
#include <iostream>

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
    backEnd(std::shared_ptr<Scope> s, std::vector<std::shared_ptr<IrNode>>* i): outerSCope(s), Ir(i){
        std::cout << "in second " << std::endl;
        outFile.open("out.s", std::ios::trunc);
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
    void movFunc(Mode mode, std::shared_ptr<Cvar> toVar,  std::shared_ptr<Cvar> fromVar);
    int findNextFree(Reg* r);
    void Printer(std::shared_ptr<Scope> s);
    void  ArithmaFunction(std::shared_ptr<ArIr> ir, bool);
    void runner(std::shared_ptr<IrNode> i, int newAdr);
    void start();
    void clearRegs(Reg* );
    void boolFunction(std::shared_ptr<getBool> b);

    Reg* cReg;
    std::set<int> delteRegs;
    std::unordered_map<std::string, int> fReturns;
    std::vector<std::shared_ptr<IrNode>>* Ir;
    int labelCounter = 0;

    Reg STMregs{9, 16};
    Reg FuRegs{0,8};
    Reg LTMregs{19,30};

    std::ofstream outFile;
    std::shared_ptr<Scope> outerSCope;
    std::shared_ptr<Scope> scope;
};