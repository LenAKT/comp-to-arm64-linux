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

void backEnd::start(){
    // int d = 0;
    // for (auto i : *Ir)
    // {
    //     if (i->instruction == Instruction::DecVar)
    //     {
    //        d++;
    //     }
    //     std::cout << toString(i->instruction) << " " << d+9 << std::endl;
    // }
    
    outFile << ".text" << std::endl;
    for(auto i : *Ir){
        switch (i->instruction)
        {
        case Instruction::DecFunc:
                makeFunction(std::static_pointer_cast<DeclareFunction>(i)->name);
            break;
        case Instruction::DecVar:{
                auto d = std::static_pointer_cast<DeclareVar>(i);
                makeVar(d->toDeclare, d->value, d->name);
            break;}
        case Instruction::RetValue:
                // forceTakeR(0);
                moveToReg(scope->Cvars["return"], 0);
                outFile << "ret" << std::endl;
            break;
        case Instruction::ChangeSc:
                scope = std::static_pointer_cast<ChangeScp>(i)->newScope;
            break;
        case Instruction::Arithma:
                ArithmaFunction(std::static_pointer_cast<ArIr>(i), std::static_pointer_cast<ArIr>(i)->isL);
            break;
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
        default:
            break;
        }
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
    if (var->reg == 99)
    {
        var->reg = findNextFree(cReg);
    }
    switch (var->value->type)
    {
    case valueType::Intager:
        movFunc(Mode::intValue, var, nullptr);
    break;
    case valueType::Varible:
        movFunc(Mode::varValue, var, scope->Cvars[std::static_pointer_cast<variableNode>(var->value)->name]);
    break;
    case valueType::Operator:
        break;
    case valueType::Func:

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
        if (r->regs[i] == false)
        {
            r->regs[i] = true;
            return i;
        }
   }
   return 99;
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

void backEnd::movFunc(Mode mode, std::shared_ptr<Cvar> toVar,  std::shared_ptr<Cvar> fromVar){
    outFile << "mov " << (toVar->is4byte ? "w" : "x") << toVar->reg << ", ";
    switch (mode)
    {
    case Mode::intValue:
            outFile << "#" << std::static_pointer_cast<intNode>(toVar->value)->value << std::endl;
        break;
    case Mode::varValue:
            if (fromVar->reg == 99)
            {
                std::cout << "order erro" << std::endl;
            }
            else{
                outFile << (toVar->is4byte ? "w" : "x") << fromVar->reg << std::endl;
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
        for (int i = 0; i < Ir->size(); i++)
        {
            if ((*Ir)[i]->instruction == Instruction::Arithma && (*Ir)[i] != ir)
            {
                auto s = std::static_pointer_cast<ArIr>((*Ir)[i]);
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
            auto s = scope->Cvars[std::static_pointer_cast<variableNode>(node)->name];
            if(type == s->var.type)
            {
                *currentReg = s->reg;
                i++;
                return true;
            }
            else{
                std::cout << "Arithmatic error 1" << std::endl;
                exit(1);
                return false;
            }
        }
        else if(node->type == valueType::Func){
            auto s = std::static_pointer_cast<functionNode>(node);
            if(type == scope->functionMap[s->name]->returnType)
            {
                *currentReg = *s->reg;
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
                    movFunc(Mode::intValue, c, nullptr);
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
        if (delteRegs.erase(reg))
        {
            cReg->regs.at(reg-cReg->start) = false;
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
   int prevReg = 99;
   bool containsOr = false;
   std::cout << "size of bool " << b->subBools.size() << std::endl;
   switch (b->type)
   {
   case BoolEnum::IF:
        for(auto i : b->subBools){
            // if (i.isBool)
            // {
            //     movFunc(Mode::intValue, i.firstArg, nullptr);
            //     outFile << "cbz w" << i.firstArg->reg << ", skip" << labelCounter << std::endl;
            //     cReg->regs[i.firstArg->reg] = false;
            // }
            if(i.firstArg->reg == 99){
                i.firstArg->reg = findNextFree(&STMregs);
                movFunc(Mode::intValue, i.firstArg, nullptr);
            }
            if(i.secondArg->reg == 99){
                i.secondArg->reg = findNextFree(&STMregs);
                movFunc(Mode::intValue, i.secondArg, nullptr);
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