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
        if (logic.collecting && token.stringValue != ";" && !((logic.setBool == true || logic.runFunction == true) && token.stringValue == ")"))
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
                }                    
            }
            else{
                cout << "didnt find var: \""  << token.stringValue << "\"" << endl;
            }
            break;
        }
        case TokenCategory::DELIMITER:
            delimiterHandlign(buffer, name, activeVar, token, i);
            break;
        case TokenCategory::KEYWORD:
            if (token.stringValue == "if" || token.stringValue == "while" || token.stringValue == "else")
            {
                std::cout << "Set bool true" << std::endl;
                name = token.stringValue;
                logic.setBool = true;
                logic.collecting = true;
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
        cvar->var = var;
    }
    else{
        isFound = true;
        cvar = d->second;
    }
    std::shared_ptr<valueNode> node;
    for (int i = 0; i < buffer.size(); i++)
    {
        std::cout << name << " " << buffer[i].stringValue << std::endl;
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
                    std::cout << "Making node for " << name  << " " << token.stringValue << std::endl;
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
            auto dt = scope->variableMap.find(token.stringValue);
            if (it != outerScope->functionMap.end())
            {
                scope->functionMap.insert(make_pair(token.stringValue, outerScope->functionMap[token.stringValue]));
                vector<preToken> b;
                int pCount = 1;
                i += 2;
                while (pCount != 0)
                {
                    if (buffer[i].stringValue == "(")
                    {
                        pCount++;
                    }
                    else if (buffer[i].stringValue == ")")
                    {
                        pCount--;
                        if (pCount != 0)
                        {
                            i++;
                        }                        
                    }
                    else{
                        b.push_back(buffer[i]);
                        i++;
                    }                  
                }
                shared_ptr<functionNode> n = functionHandling(b, it->second, name);
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
            else if(dt != scope->variableMap.end()){
                if (auto ls = dynamic_pointer_cast<operatorNode>(node))
                {
                    auto l = rDig(ls);
                    static_pointer_cast<operatorNode>(l)->r = make_shared<variableNode>(token.stringValue);
                    break;
                }
                else{
                    node = make_shared<variableNode>(token.stringValue);
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
    if (node && node->type == valueType::Operator)
    {
        auto skks = static_pointer_cast<operatorNode>(node);
        if (skks->foldCheck == false)
        {
            cvar->value = startFold(static_pointer_cast<operatorNode>(node), name, 0);   
        }
    }
    shared_ptr<DeclareVar> dVar = make_shared<DeclareVar>();
    dVar->name = name;
    dVar->toDeclare = cvar;
    dVar->value = cvar->value;
    ir.push_back(dVar);
    if (cvar->value->type == valueType::Operator)
    {
        swap(ir[ir.size()-2], ir[ir.size()-1]);
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
    if (token.stringValue == ";" && logic.setvalue == true)
    {
        getNodeTree(activeVar, buffer, name);
        buffer.clear();
        logic.setvalue = false;
        logic.collecting = false;
    }
    else if(token.stringValue == "(" && logic.runFunction == true){
        logic.collecting = true;
    }
    else if(token.stringValue == ")" && logic.runFunction == true){
        for(auto k : activeFunction->paramMap){
            shared_ptr<Cvar> c = make_shared<Cvar>();
            c->var = k.second;
            c->value = make_shared<variableNode>(k.first);
            activeFunction->scope->Cvars.insert(make_pair(k.first, c));
            activeFunction->scope->insertioOrder.push_back(k.first);
        }
        logic.collecting = false;
        logic.runFunction = false;
        buffer.clear();
    }
    else if(token.stringValue == ")" && logic.setBool == true){
        logic.collecting = false;
        if (name == "if")
        {
            std::cout << "Making bool " << std::endl;
           makeBool(buffer, BoolEnum::IF);
        }
        else if(name == "while"){
            makeBool(buffer, BoolEnum::WHILE);
        }
        else if(name == "else"){
            if (buffer[0].stringValue == "if")
            {
                buffer.erase(buffer.begin());
                makeBool(buffer, BoolEnum::Else_If);
            }
            else{
                std::shared_ptr<getBool> b = std::make_shared<getBool>();
                b->type = BoolEnum::ELSE;
                b->bscope = scope;
                b->bscope->parentS = scope;
                scope = b->bscope;
                shared_ptr<ChangeScp> cs = make_shared<ChangeScp>();
                cs->newScope = scope;
                ir.push_back(cs);
            }
        }
        buffer.clear();
    }
    if (token.stringValue == "{")
    {
       fBreaker++;
    }
    if (token.stringValue == "}")
    {
        if (logic.setBool == true)
        {
            scope = scope->parentS;
            ir.push_back(make_shared<boolReturn>());
            shared_ptr<ChangeScp> cs = make_shared<ChangeScp>();
            cs->newScope = scope;
            ir.push_back(cs);
            logic.setBool = false;
        }
        else{
            scope = activeFunction->parentScope;
            shared_ptr<ChangeScp> cs = make_shared<ChangeScp>();
            cs->newScope = activeFunction->parentScope;
            ir.push_back(cs);
        }
    }
    if (token.stringValue == ";" && logic.returningVlaue == true)
    {
        Variable p;
        p.type = activeFunction->returnType;
        getNodeTree(p ,buffer, "return");
        std::shared_ptr<ReturnValue> r = make_shared<ReturnValue>();
        ir.push_back(r);
        logic.collecting = false;
        logic.returningVlaue = false;
    }
}

void Second::makeBool(std::vector<preToken>& buffer, BoolEnum t){
    boolrig b;
    std::shared_ptr<getBool> l = std::make_shared<getBool>();
    l->type = t;
    l->bscope = make_shared<Scope>(*scope);
    l->bscope->parentS = scope;
    std::shared_ptr<ChangeScp> scp = std::make_shared<ChangeScp>();
    scope = l->bscope;
    scp->newScope = l->bscope;
    ir.push_back(scp);
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
                    setArg(scope->Cvars[token.stringValue]);        
                    b.isBool = true;
                }
                else if(dt != scope->Cvars.end()){
                    setArg(scope->Cvars[token.stringValue]);
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
}