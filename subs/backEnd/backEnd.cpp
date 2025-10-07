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
    bool run = true;
    std::shared_ptr<Scope> lastScope;
    do{
        int numberCounter;
        gIr = Ir;
        if (setStack)
        {
            for(auto i : laundry){
                (*gIr)[i.position] = std::make_shared<ArIr>(i.savee);
            }
        }
        for(auto& i : *Ir){
            lastScope = scope;
            switch (i->instruction)
            {
            case Instruction::DecFunc:
                    makeFunction(std::static_pointer_cast<DeclareFunction>(i)->name);
                break;
            case Instruction::DecVar:{
                    auto d = std::static_pointer_cast<DeclareVar>(i);
                    if (d->toDeclare->seen > 0 || d->name == "return")
                    {
                        if (d->toDeclare->sInfo.firstSeen == 0)
                        {
                            d->toDeclare->sInfo.firstSeen = numberCounter;
                        }
                         
                        makeVar(d->toDeclare, d->value, d->name);
                    }
                break;}
            case Instruction::RetValue:
                    cReg->regs[scope->Cvars["return"]->reg] = false;
                    moveToReg(scope->Cvars["return"], 0);
                    if (setStack)
                    {
                        outFile << "add sp, sp, #" << s->offset << std::endl;
                    }
                    
                    outFile << "ret" << std::endl;
                    clearRegs(&FuRegs);
                    clearRegs(&STMregs);
                break;
            case Instruction::ChangeSc:
                    lastScope = scope;
                    scope = std::static_pointer_cast<ChangeScp>(i)->newScope;
                    if(setStack){
                        if (scopeSvs[scope].set == false)
                        {
                            scopeSvs[scope].set = true;
                            scope->eCounter = scopeSvs[scope].e;
                            scope->uSkip = scopeSvs[scope].u;
                        }
                    }
                    else{
                        ScopeSaver s;
                        s.e = scope->eCounter;
                        s.u = scope->uSkip;
                        auto it = scopeSvs.find(scope);
                        if (it == scopeSvs.end())
                        {
                            scopeSvs[scope] = s;
                        }
                        
                    }
                break;
            case Instruction::Arithma:{
                    auto d = std::static_pointer_cast<ArIr>(i);
                    // if (scope->Cvars[std::static_pointer_cast<operatorNode>(d->parent)->name]->seen > 0)
                    // {
                    ArithmaFunction(std::static_pointer_cast<ArIr>(i), std::static_pointer_cast<ArIr>(i)->isL);
                    // }
                break;}
            case Instruction::getFuncVal:{
                int adr = findNextFree(cReg);
                runner(i, adr);
                auto it = std::static_pointer_cast<getFuncVal>(i);
                scope->Cvars[it->callerName]->reg = adr;
                it->reg = adr;
                moveToReg(scope->Cvars[it->callerName], adr);

                break;}
            case Instruction::getBool:
                    boolFunction(std::static_pointer_cast<getBool>(i));
                break;
            case Instruction::boolReturn:{
                if (scope->inWhile)
                {
                    outFile << "b loopStart" << scope->uSkip << std::endl;
                    labelCounter++;
                    scope->inWhile = false;
                }
                outFile << "skip" << scope->uSkip << ":" << std::endl;
                labelCounter++;
                break;
            }
            case Instruction::DestoyFunc:
                // scope->functionMap[std::static_pointer_cast<destroyFunc>(i)->name].reset();
            default:
                break;
            }
            // for(auto d : clean){
                    // std::cout << d->reg << " something important: " << d->seen << std::endl;
            // }
            cleanUp(numberCounter);
            clean.clear();
            numberCounter++;
        }
        if (spillNR != 0 && setStack == false)
        {
            std::sort(StackScores.begin(), StackScores.end(), [](auto &a, auto &b){
                return (*a.first) > (*b.first);
            });
            for(auto& i : StackScores){
                std::cout << i.second << " Score: " << (*i.first) << std::endl; 
            }
            for(auto i : delteRegs){
                if (i > 18)
                {
                    LTMregs.regs[i] = false;
                }
            }
            makeStack(lastScope);
            s->stackSetter(StackScores, spillNR+2); 
            run = false;
            for(auto i : lastScope->Cvars){
                i.second->seen = i.second->sInfo.highestSeen;
                if (i.second->reg > 18)
                {
                    LTMregs.regs[i.second->reg] = false;
                }
                i.second->reg = -1;
            }
            outFile.clear();
            outFile.str("");
            spillNR = 0;
            labelCounter = 0;
            clearRegs(&FuRegs);
            clearRegs(&STMregs);
        }
        else{
            run = true;
        }
    }
    while(!run);
    StackScores.clear();
    pushFile << outFile.str();
    outFile.str("");
    outFile.clear();
}

void backEnd::runner(std::shared_ptr<IrNode> i, int newAdr){
    auto s = std::static_pointer_cast<getFuncVal>(i);
    for (int i = 0; i < s->params.size(); i++)
    {
        forceTakeR(i);
        switch (s->params[i].cat)
        {
        case TokenCategory::NUMBER:{
            std::shared_ptr<Cvar> c = std::make_shared<Cvar>();
            c->reg = i;
            auto d =  std::make_shared<intNode>();
            d->value = std::stoi(s->params[i].stringValue);
            c->value = d;
            movFunc(Mode::intValue, c, 0);
            break;}
        case TokenCategory::VARIABLE:{
            std::string out = getRegValue(scope->Cvars[s->params[i].stringValue]);
            outFile << "mov x" << i << ", " << out << std::endl;
            cleanPush(scope->Cvars[s->params[i].stringValue]);
            break;
        }
        default:
            break;
        }
    }
    outFile << "bl " << s->name << std::endl;
    clearRegs(&FuRegs);
    clearRegs(&STMregs);
    outFile << "mov x" << newAdr << ", x0" << std::endl;
}

void backEnd::moveToReg(std::shared_ptr<Cvar> var, int reg){
    if(setStack){
        auto it = s->smap.find(var->name);
        if (it != s->smap.end())
        {
            if (var->reg != reg)
            {
                outFile << "mov " << (var->is4byte ? "w" : "x") << reg 
                << ", " << (var->is4byte ? "w" : "x") << var->reg << std::endl;
            }
            setRegVlaue(var->name);
            cReg->regs[var->reg] = false;
        }
        else if(var->reg != reg)
        {
            std::string out = getRegValue(var);
            outFile << "mov " << (var->is4byte ? "w" : "x") << reg << ", " << out << std::endl;
        }
    }
    else if(var->reg != reg){
        std::string out = getRegValue(var);
        outFile << "mov " << (var->is4byte ? "w" : "x") << reg << ", " << out << std::endl;
    }
}

void backEnd::makeVar(std::shared_ptr<Cvar> var, std::shared_ptr<valueNode> node, std::string name){
    var->value = node;
    cleanPush(var);
    if (var->reg < 0 && var->value->type != valueType::Func)
    {
        var->reg = findNextFree(cReg);
    }
    switch (var->value->type)
    {
    case valueType::Intager:
        movFunc(Mode::intValue, var, 0);
    break;
    case valueType::Varible:
        movFunc(Mode::varValue, var, scope->Cvars[std::static_pointer_cast<variableNode>(var->value)->name]);
    break;
    case valueType::Operator:
        break;
    case valueType::Func:
            if (var->reg == -1)
            {
                var->reg = *std::static_pointer_cast<functionNode>(node)->reg;
            }
            else if(std::static_pointer_cast<functionNode>(node)->ownerName != var->name){
                movFunc(Mode::varValue, var, scope->Cvars[std::static_pointer_cast<functionNode>(node)->ownerName]);
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
    if (setStack == true)
    {
            outFile << "sub sp, sp, #" << s->offset << std::endl;
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
   if (spillNR > 0)
   {
        for (int i = r->end+1; i < r->end+spillNR; i++)
        {
            if (r->regs[i] == false)
            {
                r->regs[i] = true;
                return i;
            }
        }
   }
   spillNR++;
   return r->end+spillNR;
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
    FuRegs.regs[a] = false;
}

void backEnd::movFunc(Mode mode, std::shared_ptr<Cvar> toVar,  std::shared_ptr<Cvar> fromVar){
    switch (mode)
    {
    case Mode::intValue:
            if(std::static_pointer_cast<intNode>(toVar->value)->value <= 65535){
                std::string left = getRegValue(toVar);
                outFile << "mov " << left << ", ";
                outFile << "#" << std::static_pointer_cast<intNode>(toVar->value)->value << std::endl;
            }
            else{
                std::string left = getRegValue(toVar);
                outFile << "ldr " << left << ", ";
                outFile << "=" << std::static_pointer_cast<intNode>(toVar->value)->value << std::endl;
            }            
        break;
    case Mode::varValue:{
            std::string left = getRegValue(toVar);
            std::string right = getRegValue(fromVar);
            outFile << "mov " << left << ", " << right << std::endl;
        break;}
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
                if(!setStack)
                {
                    cleanUP up;
                    up.position = i;
                    up.savee = *s;
                    laundry.push_back(up);
                }
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
                std::cout << s->second << std::endl;
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
            auto so = std::static_pointer_cast<functionNode>(node);
            if(type == scope->functionMap[so->name]->returnType)
            {
                if (setStack)
                {
                    auto it = s->omap.find(so->ownerName);
                    if (it != s->omap.end())
                    {
                        scope->Cvars[so->ownerName]->reg = pullFromStack(so->ownerName);
                    }
                }
                cleanPush(scope->Cvars[so->ownerName]);
                *currentReg = scope->Cvars[so->ownerName]->reg;
                i++;
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
            cReg->regs[reg] = false;
        }
    };
    d(leftReg);
    d(rightReg);
}

void backEnd::clearRegs(Reg* r){
    for (int i = r->start; i < r->end; i++)
    {
        r->regs[i] = false;
    }
}

//må lagast på nytt når begyn å ta med parantesar funker dårli med blanding av && å ||
void backEnd::boolFunction(std::shared_ptr<getBool> b){
    int prevReg = -1;
    bool inverted = true;
    bool useTrueValF = false;
    bool useTrueValS = false;
    int Fvalue;
    int Svalue;
   std::vector<int> delte;
   auto getVal = [&](auto iNode, bool *trueV, int *value){
        if(iNode != nullptr){cleanPush(iNode);}
        else {std::cout << "massive error 2" << std::endl;}
        if(iNode->reg < 0){
            if (iNode->value->type == valueType::Intager && std::static_pointer_cast<intNode>(iNode->value)->value < 4096)
            {
                *trueV = true;
                *value = std::static_pointer_cast<intNode>(iNode->value)->value;
            }
            else{
                iNode->reg = findNextFree(&STMregs);
                delte.push_back(iNode->reg);
                movFunc(Mode::intValue, iNode, 0);
            }
        }
   };
   switch (b->type)
   {
   case BoolEnum::WHILE:
        outFile << "loopStart" << labelCounter << ":" << std::endl;
        b->type = BoolEnum::IF;
        scope->inWhile = true;
        boolFunction(b);
        break;
   case BoolEnum::IF:
        if (scope->uSkip == -1)
        {
            scope->uSkip = labelCounter;
            labelCounter += scope->eCounter+1;
        }
        for(auto i : b->subBools){
            getVal(i.firstArg, &useTrueValF, &Fvalue);
            getVal(i.secondArg, &useTrueValS, &Svalue);
            std::string Fout = (useTrueValF ? std::to_string(Fvalue) : getRegValue(i.firstArg));
            std::string Sout = (useTrueValS ? std::to_string(Svalue) : getRegValue(i.secondArg));
            outFile << "cmp " << (useTrueValF ? "#" : "") <<  Fout << ", " << (useTrueValS ? "#" : "") 
            <<  Sout << std::endl;
            if (!i.nextCmpr.stringValue.empty())
            {
                if (i.nextCmpr.stringValue == "||")
                {
                    inverted = false;
                }
                auto g = arm64BoolOps.find(i.boolArg.stringValue);
                outFile << "b." << g->second <<  (inverted ? " skip" : " goto") << scope->uSkip << std::endl;
            }
            else{
                auto g = arm64BoolOpsInverted.find(i.boolArg.stringValue);
                if (g != arm64BoolOpsInverted.end())
                {
                    outFile << "b." << g->second << " skip" << scope->uSkip << std::endl;
                }
                else{
                    std::cout << "Big error 124 " << i.boolArg.stringValue << " " << i.reg << std::endl;
                }               
            }
        }
        if (!inverted)
        {
                outFile << "goto" << scope->uSkip << ":" << std::endl;
        }
    break;
    case BoolEnum::ELSE:
        outFile << "b skip" << std::to_string(scope->uSkip+scope->eCounter) << std::endl;
        outFile << "skip" << scope->uSkip << ":" << std::endl;
        scope->uSkip++;
        scope->eCounter--;
    break;
   default:
    break;
   }
    for(int d : delte){
        cReg->regs[d] = false;
    }
}

void backEnd::cleanUp(int i){
    for(auto& p : clean){
        bool firstSeen = false;
        if (p->sInfo.lastSeen == 0)
        {
            firstSeen = true;
            if (p->sInfo.firstSeen == 0)
            {
                 p->sInfo.firstSeen = i;
            }
            p->sInfo.lastSeen = i;
        }

        if (p->sInfo.bigestJump < i - p->sInfo.lastSeen)
        {
            p->sInfo.bigestJump = i - p->sInfo.lastSeen;
        }
        p->sInfo.lastSeen = i;
        if (p->seen <= 0)
        {
            if (p->reg <= cReg->end && p->reg >= cReg->start)
            {
                cReg->regs[p->reg] = false;
            }
        }
        p->sInfo.Score = (p->sInfo.bigestJump+1) / (p->sInfo.highestSeen + 1);
        if (firstSeen)
        {
            std::pair<float*, std::string> stackPair;
            stackPair.first = &p->sInfo.Score;
            stackPair.second = p->name;
            StackScores.push_back(stackPair);        
        }
        
    }
}

std::string backEnd::getRegValue(std::shared_ptr<Cvar> var){
    std::string out;
    if (setStack)
    {
        auto it = s->omap.find(var->name);
        if (it != s->omap.end())
        {
            auto prefix = arm64LoadOps.find(s->smap[var->name]);
            int treg;
            treg = findNextFree(cReg);
            outFile << prefix->second  << (var->is4byte ? " w" : " x") << treg << ", [sp, #" 
            <<  it->second << "]" << std::endl;
            out = (var->is4byte ? "w" : "x") + std::to_string(treg);
            cReg->regs[treg] = false;
        }
        else{
            out = (var->is4byte ? "w" : "x") + std::to_string(var->reg);
        }
    }
    else{
        out = (var->is4byte ? "w" : "x") + std::to_string(var->reg);
    }
    return out;
}

void backEnd::setRegVlaue(std::string name){
    auto dt = s->omap.find(name);
    if(dt != s->omap.end()){
        auto bt = arm64StoreOps.find(s->smap[name]);
        outFile << bt->second << " x" << scope->Cvars[name]->reg << ", [sp, #" << dt->second << "]" << std::endl;
        cReg->regs[scope->Cvars[name]->reg] = false;
    }
}

int backEnd::pullFromStack(std::string name){
    int out = findNextFree(cReg);
    auto it = s->omap.find(name);
    auto dt = s->smap.find(name);
    auto m = arm64LoadOps.find(dt->second);
    outFile << m->second << " w" << out << ", [sp, #" << it->second << "]" << std::endl;
    delteRegs.insert(out);
    return out;
}