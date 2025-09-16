#include "backEnd.h"
#include <set>

std::string toString(Instruction instr) {
    switch (instr) {
        case Instruction::Error:      return "Error";
        case Instruction::DecVar:     return "DecVar";
        case Instruction::DecFunc:    return "DecFunc";
        case Instruction::RetValue:   return "RetValue";
        case Instruction::Arithma:    return "Arithma";
        case Instruction::ChangeSc:   return "ChangeSc";
        case Instruction::getFuncVal: return "getFuncVal";
        case Instruction::getBool: return "getBool";
        case Instruction::boolReturn: return "BoolReturn";
        default: return "Unknown";
    }
}

void backEnd::Printer(std::shared_ptr<Scope> s){
    std::cout << "\nGLobal Vars: " << s->Cvars.size() << " Functions: " << s->functionMap.size() << std::endl;
    for(auto i : s->Cvars){
       if (i.second->value != nullptr)
       {
          std::cout << "    Variable: " << i.first << ", Value: ";
          i.second->value->print();
          std::cout << std::endl;
       }
    }
    for(auto i : s->functionMap){
        std::cout << "\nVariables in: " << i.first << " Params needed: ";
        for(auto o : i.second->paramMap){
            std::cout << o.first << " ";
        }
        for(auto l : i.second->scope->Cvars){
            if (l.second->value->type == valueType::Func)
            {
                std::cout << "Variable: " << l.first << ", With value: ";
                l.second->value->print();
                std::cout << " with function params: ";
                for(auto p : std::static_pointer_cast<functionNode>(l.second->value)->paramVars){
                   std::cout << p.stringValue << ", ";
                }
            }
            else{
                std::cout << "Type: " << static_cast<int>(l.second->value->type) <<" " << "Variable: " << l.first << ", With value: ";
                l.second->value->print();
            }
            std::cout << std::endl;
        }
    }

}

void backEnd::start(std::vector<std::shared_ptr<IrNode>>* Ir){
    // int d = 0;
    // for (auto i : *Ir)
    // {
    //     if (i->instruction == Instruction::DecVar)
    //     {
    //        d++;
    //     }
    //     std::cout << toString(i->instruction) << " " << d+9 << std::endl;
    // }
    gIr = Ir;
    int numberCounter;
    for(auto& i : *Ir){
        switch (i->instruction)
        {
        case Instruction::DecFunc:
                makeFunction(std::static_pointer_cast<DeclareFunction>(i)->name);
            break;
        case Instruction::DecVar:{
                auto d = std::static_pointer_cast<DeclareVar>(i);
                if (d->toDeclare->seen > 0 || d->name == "return")
                {
                     makeVar(d->toDeclare, d->value, d->name);
                }
            break;}
        case Instruction::RetValue:
                moveToReg(scope->Cvars["return"], 0);
                outFile << "ret" << std::endl;
                clearRegs(&FuRegs);
                clearRegs(&STMregs);
            break;
        case Instruction::ChangeSc:
                scope = std::static_pointer_cast<ChangeScp>(i)->newScope;
            break;
        case Instruction::Arithma:{
                auto d = std::static_pointer_cast<ArIr>(i);
                // if (scope->Cvars[std::static_pointer_cast<operatorNode>(d->parent)->name]->seen > 0)
                // {
                    std::cout << "something: " << std::static_pointer_cast<operatorNode>(d->parent)->name << std::endl;
                    ArithmaFunction(std::static_pointer_cast<ArIr>(i), std::static_pointer_cast<ArIr>(i)->isL);
                // }
            break;}
        case Instruction::getFuncVal:{
                int adr = findNextFree(cReg);
               runner(i, adr);
               std::static_pointer_cast<getFuncVal>(i)->reg = adr;
            break;}
        case Instruction::getBool:
                boolFunction(std::static_pointer_cast<getBool>(i));
              break;
        case Instruction::boolReturn:
            outFile << "skip" << labelCounter <<":" << std::endl;
            labelCounter++;
        case Instruction::DestoyFunc:
            // scope->functionMap[std::static_pointer_cast<destroyFunc>(i)->name].reset();
        default:
            break;
        }
        for( auto d : clean){
            std::cout << "something important: " << d->seen << std::endl;
        }
        cleanUp(numberCounter);
        clean.clear();
        numberCounter++;
    }
}

void backEnd::runner(std::shared_ptr<IrNode> i, int newAdr){
    auto s = std::static_pointer_cast<getFuncVal>(i);
    for (int i = 0; i < s->params.size(); i++)
    {
        if(FuRegs.regs[i] == true){forceTakeR(i);}
        switch (s->params[i].cat)
        {
        case TokenCategory::NUMBER:
            outFile << "mov x" << i << ", #" << s->params[i].stringValue << std::endl;
            break;
        case TokenCategory::VARIABLE:
            outFile << "mov x" << i << ", x" << scope->Cvars[s->params[i].stringValue]->reg << std::endl;
            break;
        default:
            break;
        }
    }
    outFile << "bl " << s->name << std::endl;
    clearRegs(&FuRegs);
    clearRegs(&STMregs);
    outFile << "mov x" << newAdr << ", x0" << std::endl;
    scope->Cvars[s->callerName]->reg = newAdr;
    FuRegs.regs[0] = false;
}

void backEnd::moveToReg(std::shared_ptr<Cvar> var, int reg){
    outFile << "mov " << (var->is4byte ? "w" : "x") << reg << ", " << (var->is4byte ? "w" : "x") << var->reg << std::endl;
}

void backEnd::makeVar(std::shared_ptr<Cvar> var, std::shared_ptr<valueNode> node, std::string name){
    var->value = node;
    if (var->reg < 0)
    {
        var->reg = findNextFree(cReg);
    }
    switch (var->value->type)
    {
    case valueType::Intager:
        movFunc(Mode::intValue, var, 0);
    break;
    case valueType::Varible:
        movFunc(Mode::varValue, var, scope->Cvars[std::static_pointer_cast<variableNode>(var->value)->name]->reg);
    break;
    case valueType::Operator:
        break;
    case valueType::Func:
        if (var->reg != *std::static_pointer_cast<functionNode>(var->value)->reg)
        {
            movFunc(Mode::varValue, var, *std::static_pointer_cast<functionNode>(var->value)->reg);
        }
        break;
    default:
        break;
    }
}

void backEnd::makeFunction(std::string name){
    if (scope->functionMap.size() > 0){cReg = &LTMregs;}
    else{cReg = &STMregs;}
    
    if (name == "main")
    {
         outFile << ".global _start\n_start:" << std::endl;
    }
    else{
        outFile << ".global " << name << "\n.type " << name << ", %function\n" << name << ":" << std::endl;
    }
    int in = 0;
    for (auto i : outerSCope->functionMap[name]->paramOrder){
        if (in < 8)
        {
            outerSCope->functionMap[name]->scope->Cvars[i]->reg = in;
            FuRegs.regs[in] = true;
            in++;
        }
        else{
            std::cout << "to many params" << std::endl;
            exit(1);
        }
    }
}

int backEnd::findNextFree(Reg* r){
   for (int i = r->start; i < r->end; i++)
   {
        std::cout << i << " " << r->regs[i] << std::endl;
        if (r->regs[i] == false)
        {
            r->regs[i] = true;
            return i;
        }
   }
   spillNR++;
   return -spillNR;
}

void backEnd::forceTakeR(int a){
    if (FuRegs.regs[a] == true)
    {
        int m = findNextFree(cReg);
        for(auto s : scope->Cvars){
            if (s.second->reg == a)
            {
                s.second->reg = m;
            }
        }
        FuRegs.regs[a] = false;
        outFile << "mov x" << m << ", x" << a << std::endl;
    }
}

void backEnd::movFunc(Mode mode, std::shared_ptr<Cvar> toVar,  int fromVar){
    outFile << "mov " << (toVar->is4byte ? "w" : "x") << toVar->reg << ", ";
    switch (mode)
    {
    case Mode::intValue:
            outFile << "#" << std::static_pointer_cast<intNode>(toVar->value)->value << std::endl;
        break;
    case Mode::varValue:
            if (fromVar < 0)
            {
                std::cout << "order erro" << std::endl;
            }
            else{
                outFile << (toVar->is4byte ? "w" : "x") << fromVar << std::endl;
            }
        break;
    default:
        break;
    }
}

void backEnd::ArithmaFunction(std::shared_ptr<ArIr> ir, bool isL){
   int toReg;
   VarType type;
    if (ir->caller == "")
    {
        toReg = findNextFree(cReg);
        type = scope->Cvars[std::static_pointer_cast<operatorNode>(ir->parent)->name]->var.type;
        std::shared_ptr<regOnlyNode> re = std::make_shared<regOnlyNode>();
        delteRegs.insert(toReg);
        re->reg = toReg;
        for (int i = 0; i < gIr->size(); i++)
        {
            if ((*gIr)[i]->instruction == Instruction::Arithma && 
                (std::static_pointer_cast<ArIr>((*gIr)[i])->l == ir->parent || 
                std::static_pointer_cast<ArIr>((*gIr)[i])->r == ir->parent))
            {
                auto s = std::static_pointer_cast<ArIr>((*gIr)[i]);
                if(isL){s->l = re;}
                else{s->r = re;}
                break;
            }
        }
    }
    else{
        toReg = scope->Cvars[ir->caller]->reg;
        type = scope->Cvars[ir->caller]->var.type;
    }
    bool isGood = false;
    int leftReg, rightReg;
    int* currentReg = &leftReg;
    int* trueVal = nullptr;
    int* trueValR = nullptr;
    int i = 0;
    auto check = [&](auto node) -> bool{
        if (node->type == valueType::Varible)
        {
            auto s = scope->Cvars.find(std::static_pointer_cast<variableNode>(node)->name);
            if(s != scope->Cvars.end() && type == s->second->var.type)
            {
                *currentReg = s->second->reg;
                i++;
                return true;
            }
            else{
                std::cout << "Arithmatic error 1 " << std::endl;
                exit(1);
                return false;
            }
        }
        else if(node->type == valueType::Func){
            auto s = std::static_pointer_cast<functionNode>(node);
            if(type == scope->functionMap[s->name]->returnType)
            {
                *currentReg = *s->reg;
                i++;
                delteRegs.insert(*s->reg);
                return true;
            }
            else{
                exit(1);
                return false;
            }
        }
        else if(node->type == valueType::Intager){
            if (type == VarType::Int)
            {   
                if (ir->c == '/' ||ir->c == '*' || ir->c == '%')
                {
                    std::shared_ptr<Cvar> c = std::make_shared<Cvar>();
                    c->value = node;
                    c->reg = findNextFree(cReg);
                    c->seen = 1;
                    movFunc(Mode::intValue, c, 0);
                    *currentReg = c->reg;
                    delteRegs.insert(c->reg);
                }
                else {
                    if (i == 0){trueVal = &std::static_pointer_cast<intNode>(node)->value;}
                    else{trueValR = &std::static_pointer_cast<intNode>(node)->value;}
                }
                i++;
                return true;
            }
            else{
                std::cout << "Arithmatic error 3" << std::endl;
                exit(1);
                return false;
            }
        }
        else if(node->type == valueType::regValue){
            *currentReg = std::static_pointer_cast<regOnlyNode>(node)->reg;   
            i++;
            return true;  
        }
        return false;
    };
    isGood = check(ir->l);
    if (isGood)
    {
        currentReg = &rightReg;
        isGood = check(ir->r);
    }
    if (trueVal != nullptr){leftReg = *trueVal;}
    if(trueValR != nullptr){rightReg = *trueValR;}
    if (isGood)
    {        
        switch (type)
        {
        case VarType::Int:{
                auto b = arm64Ops.find(ir->c);
                outFile << b->second << " w" << toReg << ", w";
                if (trueVal != nullptr && trueValR == nullptr)
                {
                    std::swap(trueVal, trueValR);
                    std::swap(leftReg, rightReg);
                }
                outFile << leftReg << ", " << ((trueValR != nullptr) ? "#" : "w") << rightReg << std::endl;
            break;}
        case VarType::String:
            break;
        default:
            break;
        }
    }
    auto d = [&](int reg){
        if (delteRegs.erase(reg) && reg > 0)
        {
            std::cout << "Delteing regs: " << reg << "at " << reg-cReg->start << std::endl;
            cReg->regs[reg] = false;
        }
    };
    d(leftReg);
    d(rightReg);
}

void backEnd::clearRegs(Reg* r){
    for (int i = 0; i < r->end-r->start; i++)
    {
        r->regs[i] = false;
    }
}


//må lagast på nytt når begyn å ta med parantesar funker dårli med blanding av && å ||
void backEnd::boolFunction(std::shared_ptr<getBool> b){
   int prevReg = -1;
   bool containsOr = false;
   std::cout << "size of bool " << b->subBools.size() << std::endl;
   switch (b->type)
   {
   case BoolEnum::IF:
        for(auto i : b->subBools){
            if(i.firstArg != nullptr){cleanPush(i.firstArg);}
            else {std::cout << "massive error" << std::endl;}
            if(i.secondArg != nullptr){cleanPush(i.secondArg);}
            else {std::cout << "massive error 2" << std::endl;}
            if(i.firstArg->reg < 0){
                i.firstArg->reg = findNextFree(&STMregs);
                movFunc(Mode::intValue, i.firstArg, 0);
            }
            if(i.secondArg->reg < 0){
                i.secondArg->reg = findNextFree(&STMregs);
                movFunc(Mode::intValue, i.secondArg, 0);
                }
            outFile << "cmp w" << i.firstArg->reg << ", w" << i.secondArg->reg << std::endl;
            if (!i.nextCmpr.stringValue.empty())
            {
                if (i.nextCmpr.stringValue == "||")
                {
                    auto g = arm64BoolOps.find(i.boolArg.stringValue);
                    outFile << "b." << g->second << " goto" << labelCounter << std::endl;
                    containsOr = true;
                }
                else if(i.nextCmpr.stringValue == "&&"){
                    auto g = arm64BoolOpsInverted.find(i.boolArg.stringValue);
                    outFile << "b." << g->second << " skip" << labelCounter << std::endl;
                }
            }
            else{
                auto g = arm64BoolOpsInverted.find(i.boolArg.stringValue);
                if (g != arm64BoolOpsInverted.end())
                {
                     outFile << "b." << g->second << " skip" << labelCounter << std::endl;
                }
                else{
                    std::cout << "Big error 124 " << i.boolArg.stringValue << std::endl;
                }               
            }

        }
    break;
   default:
    break;
   }
   if (containsOr == true)
   {
        outFile << "goto" << labelCounter << ":" << std::endl;
   }
}

void backEnd::cleanUp(int i){
    for(auto& p : clean){
        p->lastSeen = i;
        if (p->seen == 0)
        {
            cReg->regs[p->reg] = false;
        }
    }
}