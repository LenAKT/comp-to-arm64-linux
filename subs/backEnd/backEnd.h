#include "maps/maps.h"
#include <fstream>
#include <iostream>

enum class Mode{
    intValue,
    varValue,

};

class backEnd
{
    public:
    backEnd(std::shared_ptr<Scope> s, std::vector<std::shared_ptr<IrNode>>* i): scope(s), Ir(i){
        std::cout << "in second " << std::endl;
        outFile.open("out.s", std::ios::trunc);
        if (!outFile)
        {
           std::cout << "couldt open file" << std::endl;
           exit(1);
        }
        
        STMregs.resize(9, true);
        STMregs.resize(28, false);
        FuRegs.resize(8, false);
    };

    void forceTakeR(int a);

    void moveToReg(std::shared_ptr<Cvar> var, int reg);
    void makeVar(std::shared_ptr<Cvar> var,  std::shared_ptr<valueNode> node, std::string );
    void makeFunction(std::string name);
    void movFunc(Mode mode, std::shared_ptr<Cvar> toVar,  std::shared_ptr<Cvar> fromVar);
    int findNextFree(std::vector<bool>* b);
    void Printer(std::shared_ptr<Scope> s);
    void  ArithmaFunction(std::shared_ptr<ArIr> ir, bool);
    void runner(std::shared_ptr<IrNode> i);
    void start();
    void clearRegs();
    std::unordered_map<std::string, int> fReturns;
    std::vector<std::shared_ptr<IrNode>>* Ir;
    std::vector<bool> STMregs;
    std::vector<bool> FuRegs;
    std::ofstream outFile;
    std::shared_ptr<Scope> scope;
};