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
        if (logic.collecting && token.stringValue != ";" && !(logic.runFunction == true && token.stringValue == ")"))
        {
            buffer.push_back(token);
            continue;
        }
        switch (token.cat)
        {
        case TokenCategory::VARIABLE:{
            auto it = scope->variableMap.find(token.stringValue);
            auto dt = scope->functionMap.find(token.stringValue);
            if (it != scope->variableMap.end())
            {
                activeVar = it->second;
                name = token.stringValue;
            }
            else if(dt != scope->functionMap.end())
            {
                if (dt->second->scope != nullptr)
                {
                    shared_ptr<DeclareFunction> df = make_shared<DeclareFunction>();
                    df->name = token.stringValue;
                    ir.push_back(df);
                    shared_ptr<ChangeScp> cs = make_shared<ChangeScp>();
                    cs->newScope = dt->second->scope;
                    ir.push_back(cs);
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
            auto it = scope->functionMap.find(token.stringValue);
            auto dt = scope->variableMap.find(token.stringValue);
            if (it != scope->functionMap.end())
            {
                vector<preToken> b;
                int pCount = 1;
                i += 2;
                while (pCount != 0)
                {
                    if (buffer[i].stringValue == "(")
                    {
                        pCount++;
                        i++;
                    }
                    else if (buffer[i].stringValue == ")")
                    {
                        pCount--;
                        i++;
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
                    static_pointer_cast<operatorNode>(l)->r = make_shared<variableNode>(token.stringValue);
                }
                else{
                    node = n;
                }
            }
            else if(dt != scope->variableMap.end()){
                if (auto ls = dynamic_pointer_cast<operatorNode>(node))
                {
                    auto l = rDig(ls);
                    static_pointer_cast<operatorNode>(l)->r = make_shared<variableNode>(token.stringValue);
                }
                else{
                    node = make_shared<variableNode>(token.stringValue);
                }
            }
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
    std::cout << "Declaring: " << name << std::endl;
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
        f->paramVars.push_back(i);
    }
    f->name = n->name;

    std::shared_ptr<getFuncVal> a = make_shared<getFuncVal>();
    a->name = f->name;
    a->params = f->paramVars;
    a->callerName = name;
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
    if (token.stringValue == "{")
    {
       fBreaker++;
    }
    if (token.stringValue == "}")
    {
        fBreaker--;
       if (fBreaker == 0)
       {
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