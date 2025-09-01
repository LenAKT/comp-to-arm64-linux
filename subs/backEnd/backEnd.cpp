#include "backEnd.h"

std::string toString(Instruction instr) {
    switch (instr) {
        case Instruction::Error:      return "Error";
        case Instruction::DecVar:     return "DecVar";
        case Instruction::DecFunc:    return "DecFunc";
        case Instruction::RetValue:   return "RetValue";
        case Instruction::Arithma:    return "Arithma";
        case Instruction::ChangeSc:   return "ChangeSc";
        case Instruction::getFuncVal: return "getFuncVal";
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
    int d = 0;
    for (auto i : *Ir)
    {
        if (i->instruction == Instruction::DecVar)
        {
           d++;
        }
        std::cout << toString(i->instruction) << " " << d+9 << std::endl;
    }
    
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
                forceTakeR(0);
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
               runner(i);
            break;}
        default:
            break;
        }
    }
}

void backEnd::runner(std::shared_ptr<IrNode> i){
     auto s = std::static_pointer_cast<getFuncVal>(i);
    for (int i = 0; i < s->params.size(); i++)
    {
        forceTakeR(i);
        switch (s->params[i].cat)
        {
        case TokenCategory::NUMBER:
            outFile << "mov x" << i << ", #" << s->params[i].stringValue << std::endl;
            break;
        case TokenCategory::VARIABLE:
            outFile << "mov x" << i << ", x" << scope->Cvars[s->params[i].stringValue]->reg << std::endl;
        default:
            break;
        }
    }
    outFile << "bl " << s->name << std::endl;
    int newAdr = findNextFree(&STMregs);
    outFile << "mov x" << newAdr << ", x0" << std::endl;
    scope->Cvars[s->callerName]->reg = newAdr;
    FuRegs[0] = false;
}

void backEnd::moveToReg(std::shared_ptr<Cvar> var, int reg){
    outFile << "mov " << (var->is4byte ? "w" : "x") << reg << ", " << (var->is4byte ? "w" : "x") << var->reg << std::endl;
}

void backEnd::makeVar(std::shared_ptr<Cvar> var, std::shared_ptr<valueNode> node, std::string name){
    var->reg = findNextFree(&STMregs);
    var->value = node;
    switch (var->value->type)
    {
    case valueType::Intager:
        movFunc(Mode::intValue, var, nullptr);
    break;
    case valueType::Varible:
        movFunc(Mode::varValue, var, scope->Cvars[std::static_pointer_cast<variableNode>(var->value)->name]);
    break;
    case valueType::Operator:{
        // auto p = std::static_pointer_cast<operatorNode>(var->value);
        // std::shared_ptr<ArIr> arIr = std::make_shared<ArIr>();
        // arIr->c = p->c;
        // arIr->l = p->l;
        // arIr->caller = name;
        // arIr->r = p->r;
        // ArithmaFunction(arIr);
        break;}
    default:
        break;
    }
}

void backEnd::makeFunction(std::string name){
    if (name == "main")
    {
         outFile << ".global _start\n_start:" << std::endl;
    }
    else{
        outFile << ".global " << name << "\n.type " << name << ", %function\n" << name << ":" << std::endl;
    }
    int in = 0;
    for (auto i : scope->functionMap[name]->paramOrder){
        if (in < 8)
        {
            scope->functionMap[name]->scope->Cvars[i]->reg = in;
            FuRegs[in] = true;
            in++;
        }
        else{
            std::cout << "to many params" << std::endl;
            exit(1);
        }
    }
}

int backEnd::findNextFree(std::vector<bool>* r){
   for (int i = 0; i < r->size(); i++)
   {
        if ((*r)[i] == false)
        {
            (*r)[i] = true;
            return i;
        }
   }
   return 99;
}

void backEnd::forceTakeR(int a){
    if (FuRegs[a] == true)
    {
        int m = findNextFree(&STMregs);
        for(auto s : scope->Cvars){
            if (s.second->reg == a)
            {
                s.second->reg = m;
            }
        }
        FuRegs[a] = false;
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
        toReg = findNextFree(&STMregs);
        type = scope->Cvars[std::static_pointer_cast<operatorNode>(ir->parent)->name]->var.type;
        std::shared_ptr<regOnlyNode> re = std::make_shared<regOnlyNode>();
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
                forceTakeR(0);
                std::shared_ptr<getFuncVal> v = std::make_shared<getFuncVal>();
                v->name = s->name;
                v->params = s->paramVars;
                v->callerName = "void";
                runner(v);
                *currentReg = 0;
                return true;
            }
            else{
                std::cout << "Arithmatic error 2" << std::endl;
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
                    c->reg = findNextFree(&STMregs);
                    movFunc(Mode::intValue, c, nullptr);
                    *currentReg = c->reg;
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
                outFile << ((trueVal != nullptr) ? *trueVal : leftReg)
                << ", " << ((trueValR != nullptr) ? "#" : "w") << ((trueValR != nullptr) ? *trueValR : rightReg) << std::endl;
            break;}
        case VarType::String:
            break;
        default:
            break;
        }
    }
}