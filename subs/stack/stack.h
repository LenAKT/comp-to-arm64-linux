#include "maps/maps.h"
#include <vector>
#include <fstream>



class stackHandler
{
    stackHandler(std::ofstream& f) : outFile(f) {};

    void setFunctionStack(std::shared_ptr<Scope> s);
    
    std::ofstream& outFile;
};

