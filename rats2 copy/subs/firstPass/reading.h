#include <fstream>
#include <iostream>
#include "maps/maps.h"

using namespace std;


class reading
{
    public:
    void firstPass();
    void setMemory(string path);
    void reader();
    void printer();
    void functionAssigning(shared_ptr<NFunction> nFunction, std::vector<preToken>& buffer);
    void variableAssigning(std::unordered_map<std::string, Variable>& variableMap, std::unordered_map<std::string,shared_ptr<NFunction>>& functionMap, std::vector<preToken>& pretokens);

    shared_ptr<NFunction> nFunction;
    std::vector<std::string> order;
    shared_ptr<Scope> scope = make_shared<Scope>();
    std::vector<preToken> preTokens;
    private:
    string mainMemory;
};


