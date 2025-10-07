#include "stack.h"
#include <unordered_map>


void stackHandler::stackSetter(std::vector<std::pair<float*, std::string>>& in, int nr){
    for (int i = 0; i < nr; i++)
    {
        size_t size = getSize(scope->Cvars[in[i].second]);
        smap[in[i].second] = size;
        omap[in[i].second] = offset;
        offset += size;
    }
    if (offset % 16 != 0)
    {
        offset = offset + 16 - (offset % 16);
    }
}

size_t stackHandler::getSize(std::shared_ptr<Cvar> cvar){
    switch (cvar->var.type)
    {
    case VarType::Int:
        return 4;
        break;
    case VarType::Char:
        return 2;
        break;
    case VarType::Bool:
        return 1;
        break;
    case VarType::Float:
        return 4;
        break;
    case VarType::Pointer:
        return 8;
    default:
        break;
    }
    return 0;
}