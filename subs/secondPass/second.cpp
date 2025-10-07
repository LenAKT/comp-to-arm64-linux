#include "second.h"
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>

using namespace std;



void Second::secondPass(){
    Variable activeVar;
    vector<preToken> buffer;
    std::string name; 

    for (int i = 0; i < preTokens.size(); i++)
    {
        auto& token = preTokens[i];
        if (logic.collecting && token.stringValue != ";" && !(logic.inParams && token.stringValue == ")"))
        {
            buffer.push_back(token);
            continue;
        }
        switch (token.cat)
        {
        case TokenCategory::VARIABLE:{
            auto it = scope->variableMap.find(token.stringValue);
            auto dt = outerScope->functionMap.find(token.stringValue);
            if (it != scope->variableMap.end())
            {
                activeVar = it->second;
                name = token.stringValue;
            }
            else if(dt != outerScope->functionMap.end())
            {
                if (dt->second->scope != nullptr)
                {
                    shared_ptr<ChangeScp> cs = make_shared<ChangeScp>();
                    cs->newScope = dt->second->scope;
                    ir.push_back(cs);

                    shared_ptr<DeclareFunction> df = make_shared<DeclareFunction>();
                    df->name = token.stringValue;
                    ir.push_back(df);

                    scope = dt->second->scope;
                    activeFunction =  dt->second;
                    logic.runFunction = true;
                    logic.inParams = true;
                }                    
            }
            else{
                cout << "didnt find var: \""  << token.stringValue << "\"" << endl;
            }
            break;
        }
        case TokenCategory::DELIMITER:
            delimiterHandlign(buffer, name, activeVar, token, i);
            if (logic.setBool == 1)
            {
                if (preTokens.size() > i && preTokens[i+1].stringValue == "else")
                {
                    std::cout << name << std::endl;
                    break;
                }
                else{
                    logic.setBool = 0;
                    scope = scope->parentS;
                    shared_ptr<ChangeScp> s = make_shared<ChangeScp>();
                    s->newScope = scope;
                    ir.push_back(s);
                }
            }
            break;
        case TokenCategory::KEYWORD:
            if (token.stringValue == "if" || token.stringValue == "while" || token.stringValue == "else")
            {
                name = token.stringValue;
                if (logic.setBool == 0)
                {
                    shared_ptr<ChangeScp> sc = make_shared<ChangeScp>();
                    sc->newScope = make_shared<Scope>(*scope);
                    sc->newScope->uSkip = -1;
                    sc->newScope->eCounter = 0;
                    sc->newScope->parentS = scope;
                    scope = sc->newScope;
                    ir.push_back(sc);
                    logic.setBool++;
                }
                logic.setBool++;
                if (token.stringValue != "else")
                {
                    logic.inParams = true;
                }
                else {
                    boolhandling(buffer, name, token);
                }
            } 
            if (token.stringValue == "return")
            {
                logic.returningVlaue = true;
                logic.collecting = true;
                buffer.clear();
            }
            break;
        case TokenCategory::OPERATOR:
            if (token.stringValue == "=")
            {
                logic.setvalue = true;
                logic.collecting = true;
            }
            break;
        default:
            break;
        }
    }
}

void Second::getNodeTree(Variable var,vector<preToken>& buffer, std::string name){
    shared_ptr<Cvar> cvar;
    bool isFound = false;
    auto d = scope->Cvars.find(name);
    if (d == scope->Cvars.end())
    {   
        cvar = make_shared<Cvar>();
        if (name == "return")
        {
            cvar->seen = 1;
        }
        cvar->var = var;
    }
    else{
        isFound = true;
        cvar = d->second;
        cvar->seen++;
    }
    std::shared_ptr<valueNode> node;
    for (int i = 0; i < buffer.size(); i++)
    {
        auto& token = buffer[i];
        switch (token.cat)
        {
        case TokenCategory::NUMBER:
            node = assignValue<intNode>(node, stoi(token.stringValue));
            break;
        case TokenCategory::DELIMITER:{
            
            break;
            }
        case TokenCategory::OPERATOR:{
            auto it = operatorMap.find(token.stringValue);
            if (it != operatorMap.end())
            {
                if (auto lt = dynamic_pointer_cast<operatorNode>(node))
                {
                    if (token.stringValue == "/"  || token.stringValue == "*" || token.stringValue == "%")
                    {
                        auto opnode = make_shared<operatorNode>();
                        opnode->name = name;
                        opnode->c = token.stringValue[0];
                        opnode->l = lt->r;
                        lt->r = opnode;
                    }
                    else{
                        auto opnode = make_shared<operatorNode>();
                        opnode->name = name;
                        opnode->c = token.stringValue[0];
                        opnode->l = lt;
                        node = opnode;
                    }
                }
                else{
                    auto opnode = make_shared<operatorNode>();
                    opnode->name = name;
                    opnode->l = node;
                    opnode->c = token.stringValue[0];
                    node = opnode;
                }
            }
            break;
        }
        case TokenCategory::STRING:
            node = assignValue<stringNode>(node, token.stringValue);
            break;
        case TokenCategory::VARIABLE:{
            auto it = outerScope->functionMap.find(token.stringValue);
            auto dt = scope->Cvars.find(token.stringValue);
            if (it != outerScope->functionMap.end())
            {
                scope->functionMap.insert(make_pair(token.stringValue, outerScope->functionMap[token.stringValue]));
                vector<preToken> b = paramCollector(buffer, &i, it->second);
                shared_ptr<functionNode> n = functionHandling(b, it->second, name);
                n->ownerName = name;
                cvar->seen++;
                if (auto ls = dynamic_pointer_cast<operatorNode>(node))
                {
                    auto l = rDig(ls);
                    static_pointer_cast<operatorNode>(l)->r = n;
                    break;
                }
                else{
                    node = n;
                    break;
                }
            }
            else if(dt != scope->Cvars.end()){
                if (dt->second->value->type != valueType::Intager)
                {
                    dt->second->seen++;
                }
                if (auto ls = dynamic_pointer_cast<operatorNode>(node))
                {
                    auto l = rDig(ls);
                    static_pointer_cast<operatorNode>(l)->r = dt->second->value;
                    break;
                }
                else{
                    if (dt->second->value->type == valueType::Operator)
                    {
                        node = make_shared<variableNode>(dt->first);
                    }
                    else{
                        node = dt->second->value;
                    }
                    break;
                }
            }
            break;
        }
        default:
            break;
        }
    }
    cvar->value = node;
    cvar->name = name;
    if (node && node->type == valueType::Operator)
    {
        auto skks = static_pointer_cast<operatorNode>(node);
        if (skks->foldCheck == false)
        {
            cvar->value = startFold(static_pointer_cast<operatorNode>(node), name, 0);
        }
    }
    if (isFound == false || cvar->value->type != valueType::Intager || logic.setBool > 0)
    {
        shared_ptr<DeclareVar> dVar = make_shared<DeclareVar>();
        dVar->name = name;
        dVar->toDeclare = cvar;
        dVar->value = cvar->value;
        ir.push_back(dVar);
    }
    else if(cvar->value->type == valueType::Intager){
       for (auto& d : ir){
            if (d->instruction == Instruction::DecVar)
            {
                if ( static_pointer_cast<DeclareVar>(d)->name == name)
                {
                    static_pointer_cast<DeclareVar>(d)->value = cvar->value;
                }
            }
       }
    }  
    if (cvar->value->type == valueType::Operator)
    {
        swap(ir[ir.size()-2], ir[ir.size()-1]);
        cvar->value = std::make_shared<variableNode>(name);
    }
    if (logic.setBool > 0 && isFound == true)
    {
        cvar->value = make_shared<variableNode>(name);
    }
    scope->Cvars.insert(make_pair(name, std::move(cvar)));
    scope->insertioOrder.push_back(name);
}

template<typename nodeType, typename valueType>
shared_ptr<valueNode> Second::assignValue(shared_ptr<valueNode>& node, valueType tvalue){
   auto newNode = make_shared<nodeType>();
    std::static_pointer_cast<nodeType>(newNode)->value = tvalue;
   if (auto it = dynamic_pointer_cast<operatorNode>(node))
   {    
        auto l = rDig(it);
        static_pointer_cast<operatorNode>(l)->r = newNode;
       
   }
   else if(node == nullptr || node->type == newNode->type)
   {
        node = newNode;
   }
   return node;
}

shared_ptr<valueNode> Second::startFold(const shared_ptr<operatorNode>& v, std::string pName, bool c){
    auto bt = dynamic_pointer_cast<operatorNode>(v->l);
    auto lt = dynamic_pointer_cast<operatorNode>(v->r);
    if(bt)
    {
        v->l = startFold(static_pointer_cast<operatorNode>(v->l), "", 1);
    }
    if(lt)
    {
        v->r = startFold(static_pointer_cast<operatorNode>(v->r), "", 0);
    }
    if (v->r != nullptr && v->l->type == v->r->type)
    {
        if (auto it = dynamic_pointer_cast<intNode>(v->l))
        {
            return Intfolder(v);
        }
        else if (auto dt = dynamic_pointer_cast<stringNode>(v->l))
        {
            return stringfolder(v);
        }
    }
    if (v->l != nullptr && v->r != nullptr)
    {
        shared_ptr<ArIr> arIr = make_shared<ArIr>();   
        arIr->c = v->c;
        arIr->parent = v;
        arIr->l = v->l;
        arIr->r = v->r;
        arIr->caller = pName;
        arIr->isL = c;
        ir.push_back(arIr);
    }
   
    if (auto it = dynamic_pointer_cast<operatorNode>(v))
    {
        it->foldCheck = true;
    }
    return v;
}

shared_ptr<valueNode> Second::rDig(shared_ptr<operatorNode>& n){
    if (n->r != nullptr && n->r->type == valueType::Operator)
    {
        auto it = static_pointer_cast<operatorNode>(n->r);
        return rDig(it);
    }
    else{
        return n;
    }
}

shared_ptr<valueNode> Second::Intfolder(const shared_ptr<operatorNode>& node){
    shared_ptr<intNode> returnNode = make_shared<intNode>();
    switch (node->c)
    {
    case '+':
        returnNode->value = static_pointer_cast<intNode>(node->l)->value + static_pointer_cast<intNode>(node->r)->value;
        break;
    case '-':
        returnNode->value = static_pointer_cast<intNode>(node->l)->value - static_pointer_cast<intNode>(node->r)->value;
        break;
    case '*':
        returnNode->value = static_pointer_cast<intNode>(node->l)->value * static_pointer_cast<intNode>(node->r)->value;
        break;
    case '%':
        returnNode->value = static_pointer_cast<intNode>(node->l)->value % static_pointer_cast<intNode>(node->r)->value;
        break;
    case '/':
        returnNode->value = static_pointer_cast<intNode>(node->l)->value / static_pointer_cast<intNode>(node->r)->value;
        break;
    }
    return returnNode;
}

shared_ptr<valueNode> Second::stringfolder(const shared_ptr<operatorNode>& node){
    shared_ptr<stringNode> returnNode = make_shared<stringNode>();
    switch (node->c)
    {
    case '+':
        returnNode->value = static_pointer_cast<stringNode>(node->l)->value += static_pointer_cast<stringNode>(node->r)->value;
        break;
    default:
        cout << "Wrong operator for strings";
    }
    return returnNode;
}

std::shared_ptr<functionNode>  Second::functionHandling(vector<preToken>& buffer, shared_ptr<NFunction>& n, std::string name)
{
    std::shared_ptr<functionNode> f = std::make_shared<functionNode>();
    f->type = valueType::Func;
    for(auto i : buffer){
        if (i.cat == TokenCategory::NUMBER || i.cat == TokenCategory::VARIABLE)
        {
           f->paramVars.push_back(i);
           if (i.cat == TokenCategory::VARIABLE)
           {
              auto it = scope->Cvars.find(i.stringValue);
              if (it != scope->Cvars.end())
              {
                it->second->seen++;
              }
           }
        }
    }
    f->name = n->name;
    std::shared_ptr<getFuncVal> a = make_shared<getFuncVal>();
    a->name = f->name;
    a->params = f->paramVars;
    a->callerName = name;
    f->reg = &a->reg;
    ir.push_back(a);
    return f;
}

void Second::delimiterHandlign(vector<preToken>& buffer, std::string& name, Variable& activeVar,const preToken& token, int& i){
    if (logic.runFunction == true && logic.inParams && logic.setBool == 0)
    {
        functionDealer(buffer, name, activeVar, token, i);
    }
    if (logic.setBool > 0 && logic.inParams)
    {
        boolhandling(buffer, name, token);
    }
    if (token.stringValue == ";" && logic.setvalue == true && logic.returningVlaue == false)
    {
        getNodeTree(activeVar, buffer, name);
        buffer.clear();
        logic.setvalue = false;
        logic.collecting = false;
    }
    if (token.stringValue == "{")
    {
       fBreaker++;
    }
    if (token.stringValue == "}")
    {
        shared_ptr<ChangeScp> cs = make_shared<ChangeScp>();
        cs->newScope = scope->parentS;
        ir.push_back(cs);
        if (logic.runFunction == true && logic.setBool == 0)
        {
            startIR(scope);
            logic.runFunction = false;
            scope = scope->parentS;
        }   
        else if (logic.setBool > 0)
        {
            logic.setBool--;
            ir.push_back(make_shared<boolReturn>());
            scope = scope->parentS;
        }
    }
    if (token.stringValue == ";" && logic.returningVlaue == true)
    {
        Variable p;
        p.type = activeFunction->returnType;
        getNodeTree(p ,buffer, "return");
        std::shared_ptr<ReturnValue> r = make_shared<ReturnValue>();
        ir.push_back(r);
        buffer.clear();
        logic.collecting = false;
        logic.returningVlaue = false;
    }
}

void Second::makeBool(std::vector<preToken>& buffer, BoolEnum t){
    boolrig b;
    std::shared_ptr<getBool> l = std::make_shared<getBool>();
    l->type = t;
    auto setArg = [&] (std::shared_ptr<Cvar> c){
        if (b.firstArg == nullptr){b.firstArg = c;}
        else{b.secondArg = c;}
    };
    for (int i = 0; i < buffer.size(); i++)
    {   
        auto& token = buffer[i];
        switch (token.cat)
        {
        case TokenCategory::VARIABLE:{
            auto dt = scope->Cvars.find(token.stringValue);
                if (dt != scope->Cvars.end() && scope->Cvars[token.stringValue]->var.type == VarType::Bool)
                {
                    dt->second->seen++;
                    setArg(scope->Cvars[token.stringValue]);        
                    b.isBool = true;
                }
                else if(dt != scope->Cvars.end()){
                    dt->second->seen++;
                    setArg(scope->Cvars[token.stringValue]);
                }
            auto ls = scope->functionMap.find(token.stringValue);
                if (ls != scope->functionMap.end())
                {
                    std::shared_ptr<getFuncVal> a = make_shared<getFuncVal>();
                    std::shared_ptr<regOnlyNode> ba = make_shared<regOnlyNode>();
                    ba->pReg = &a->reg;
                    a->callerName = "v" + to_string(scope->emptyCounter);
                    scope->emptyCounter++;
                    a->name = token.stringValue;
                    a->params = paramCollector(buffer, &i, nullptr);
                    ir.push_back(a);
                    std::shared_ptr<Cvar> tihi = std::make_shared<Cvar>();
                    tihi->value = ba;
                    setArg(tihi);
                }
            }
            break;
        case TokenCategory::NUMBER:{
                std::shared_ptr<intNode> n = std::make_shared<intNode>();
                n->value = stoi(token.stringValue);
                std::shared_ptr<Cvar> tihi = std::make_shared<Cvar>();
                tihi->value = n;
                setArg(tihi);
            }
            break;
        case TokenCategory::BOOLOP:
                b.nextCmpr = token;
                l->subBools.push_back(b);
                b = boolrig{};
            break;
        case TokenCategory::OPERATOR:
                b.boolArg = token;
            break;
        default:
            break;
        }
    }
    l->subBools.push_back(b);
    ir.push_back(l);
    if (ir.size() > 3 && ir[ir.size()-3]->instruction == Instruction::getBool)
    {
       auto it = static_pointer_cast<getBool>(ir[ir.size()-3]);
       if (it->type == BoolEnum::ELSE)
       {
            logic.setBool--;
            l->bscope = it->bscope;
            scope = l->bscope;
            swap(ir[ir.size()-2], ir[ir.size()-1]);
            return;
       }
    }
    std::shared_ptr<ChangeScp> scp = std::make_shared<ChangeScp>();
    l->bscope = make_shared<Scope>(*scope);
    l->bscope->uSkip = -1;
    l->bscope->eCounter = 0;
    l->bscope->parentS = scope;
    scope = l->bscope;
    scp->newScope = l->bscope;
    ir.push_back(scp);
}

void Second::functionDealer(vector<preToken>& buffer, std::string& name, Variable& activeVar,const preToken& token, int& i){
    if(token.stringValue == "("){
        logic.collecting = true;
    }
    else if(token.stringValue == ")" ){
        for(auto k : activeFunction->paramMap){
            shared_ptr<Cvar> c = make_shared<Cvar>();
            c->var = k.second;
            c->seen = 0;
            c->value = make_shared<variableNode>(k.first);
            activeFunction->scope->Cvars.insert(make_pair(k.first, c));
            activeFunction->scope->insertioOrder.push_back(k.first);
        }
        logic.collecting = false;
        logic.inParams = false;
        buffer.clear();
    }
}

void Second::boolhandling(vector<preToken>& buffer, std::string& name,const preToken& token){
    if(token.stringValue == ")" || name == "else"){
        logic.collecting = false;
        if (name == "if")
        {
           makeBool(buffer, BoolEnum::IF);
        }
        else if(name == "while"){
            makeBool(buffer, BoolEnum::WHILE);
        }
        else if(name == "else"){
            ir.pop_back();
            scope->eCounter++;
            std::shared_ptr<getBool> b = std::make_shared<getBool>();
            b->type = BoolEnum::ELSE;
            b->bscope = make_shared<Scope>(*scope);
            b->bscope->uSkip = -1;
            b->bscope->eCounter = 0;
            b->bscope->parentS = scope;
            ir.push_back(b);
            shared_ptr<ChangeScp> c = make_shared<ChangeScp>();
            c->newScope = b->bscope;
            scope = b->bscope;
            ir.push_back(c);
        }
        buffer.clear();
        logic.inParams = false;
    }
    else if(token.stringValue == "(" && logic.setBool > 0){
        logic.collecting = true;
    }
}

std::vector<preToken> Second::paramCollector(std::vector<preToken>& buffer, int *position, shared_ptr<NFunction> n){
    std::vector<preToken> ret;
    std::vector<std::vector<preToken>> bTwo;
    std::vector<preToken> out;
    bool nest = false;
    int pCount = 1;
    int control = 0;
    while (buffer[*position].stringValue != "(")
    {
        (*position)++;
        control++;
        if (control > 10)
        {
            std::cout << "error 500" << std::endl;
            exit(1);
        }
    }
    (*position)++;
    while (pCount != 0)
    {
        if (buffer[*position].stringValue == "(")
        {
            nest = true;
            pCount++;
            ret.push_back(buffer[*position]);
            (*position)++;
        }
        else if (buffer[*position].stringValue == ")")
        {
            nest = false;
            pCount--;
            if (pCount != 0)
            {
                ret.push_back(buffer[*position]);    
                (*position)++;
            }                        
        }
        else if(buffer[*position].stringValue == "," && nest == false){
            bTwo.push_back(ret);
            ret.clear();
            (*position)++;
        }
        else{
            ret.push_back(buffer[*position]);    
            (*position)++;
        }    
    }
    bTwo.push_back(ret);
    for(int i = 0; i < bTwo.size(); i++){
        if (bTwo[i].size() > 1)
        {
            Variable p;
            p = n->paramMap[n->paramOrder[i]];
            string n_ame = "param" + to_string(fakeNum);
            fakeNum++;
            getNodeTree(p, bTwo[i], n_ame);
            preToken aliasT;
            aliasT.cat = TokenCategory::VARIABLE;
            aliasT.stringValue = n_ame;
            out.push_back(aliasT);
        }
        else{
            out.push_back(bTwo[i][0]);
        }
    }
    return out;
}

void Second::startIR(std::shared_ptr<Scope> scope){
    backend.start(irSender);
    activeFunction->scope = nullptr;
    ir.clear();
    std::cout << "in First" << std::endl;
}