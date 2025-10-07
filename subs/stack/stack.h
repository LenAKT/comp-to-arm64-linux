#include "maps/maps.h"
#include <vector>
#include <sstream>
#include <iostream>
#include <unordered_map>

#pragma once

class stackHandler
{
    public:
    stackHandler(std::shared_ptr<Scope> s) : scope(s) {};
    std::shared_ptr<Scope> scope;
    std::unordered_map<std::string, int> smap;
    std::unordered_map<std::string, int> omap;
    bool ready = false;
    size_t offset = 0;
    void stackSetter(std::vector<std::pair<float*, std::string>>& in, int nr);
    size_t getSize(std::shared_ptr<Cvar>);
};

