#include "stack.h"


void stackHandler::setFunctionStack(std::shared_ptr<Scope> scope){

    size_t stackSize = 0;

    for(auto& var : scope->Cvars){
        if (var.second->is4byte == true)
        {
            stackSize += 4;
        }
        else if(var.second->is4byte == false){
            stackSize += 8;
        }
    }
}