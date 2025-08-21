#include "second.h"
#include <iostream>
#include <vector>
#include <set>

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
    shared_ptr<Cvar> cvar = make_shared<Cvar>();
    cvar->var = var;
    bool delayedRAssign = false;
    std::shared_ptr<valueNode> node ;
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
                        opnode->c = token.stringValue[0];
                        opnode->l = lt->r;
                        lt->r = opnode;
                        delayedRAssign = true;
                    }
                    else{
                        auto opnode = make_shared<operatorNode>();
                        opnode->c = token.stringValue[0];
                        opnode->l = lt;
                        node = opnode;
                    }
                }
                else{
                    auto opnode = make_shared<operatorNode>();
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
                cout << "Found func" << token.stringValue << endl;
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

                functionHandling(b, it->second);
                if (auto ls = dynamic_pointer_cast<operatorNode>(node))
                {
                    ls->r = valuePasser;
                }
                else{
                    node = valuePasser;
                }
                
            }
            else if(dt != scope->variableMap.end()){
                auto cv = scope->Cvars.find(token.stringValue);
                if (cv != scope->Cvars.end())
                {
                    if (auto ls = dynamic_pointer_cast<operatorNode>(node))
                    {
                        ls->r = scope->Cvars[token.stringValue]->value;
                    }
                    else{
                        node = scope->Cvars[token.stringValue]->value;
                    }
                }
                else{
                   if (auto ls = dynamic_pointer_cast<operatorNode>(node))
                    {
                        ls->r = make_shared<variableNode>(token.stringValue);
                    }
                    else{
                        node = make_shared<variableNode>(token.stringValue);
                    }
                }
            }
        }
        default:
            break;
        }
    }
    cvar->value = node;
    cout << "Making new active var: " << name << " type: " << static_cast<int>(cvar->var.type) << " Value: "; 
    cvar->value->print();
    cout << endl;
    cout << "vector sieze: " << scope->Cvars.size() << endl;
    if (node->type == valueType::Operator)
    {
        cvar->value = startFold(static_pointer_cast<operatorNode>(node));
        cout << "New value for : " << name << " is ";
        cvar->value->print();
        cout << endl;
    }
    if (scope->Cvars.size() > 1000000) {
    std::cerr << "POSSIBLE CORRUPTION: Cvars too large: " << scope->Cvars.size() << std::endl;
    exit(1);
}
    cout << endl;
    //pass down value to ir differentiate between "true" variables and variable names or entire funtion calls that need later interpritation? that need later interpritation
    scope->Cvars.insert(make_pair(name, std::move(cvar)));
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

shared_ptr<valueNode> Second::startFold(const shared_ptr<operatorNode>& v){
    auto bt = dynamic_pointer_cast<operatorNode>(v->l);
    auto lt = dynamic_pointer_cast<operatorNode>(v->r);
    if(bt)
    {
        v->l = startFold(static_pointer_cast<operatorNode>(v->l));
    }

    if(lt)
    {
        v->r = startFold(static_pointer_cast<operatorNode>(v->r));
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
    else{
        cout << "couldt fold" << endl;
        return v;
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

std::vector<preToken> Second::makeParam(std::vector<preToken>& incValue, vector<preToken>& names, unordered_map<std::string, Variable>& k){
    std::vector<preToken> pushBack;
    std::vector<preToken> inventory;
    set<string> seen;
    for(auto n : names){
        if (n.cat == TokenCategory::VARIABLE)
        {
            auto dt = k.find(n.stringValue);
            if (dt != k.end())
            {
                if (seen.insert(n.stringValue).second)
                {
                    inventory.push_back(n);
                }
            }
        }
    }
    int q = 0;
    for(auto i : inventory){
        
        preToken p;
        pushBack.push_back(i);
        p.cat = TokenCategory::OPERATOR;
        p.stringValue = "=";
        pushBack.push_back(p);
        while (q < incValue.size())
        {
            if (incValue[q].cat != TokenCategory::DELIMITER)
            {
                pushBack.push_back(incValue[q]);
                q++;
            }
            else{
                p.cat = TokenCategory::DELIMITER;
                p.stringValue = ";";
                pushBack.push_back(p);
                return pushBack;
                q++;
                break;
            }
        }
    }
    preToken p;
    p.cat = TokenCategory::DELIMITER;
    p.stringValue = ";";
    pushBack.push_back(p);
    return pushBack;
}

void Second::functionHandling(vector<preToken>& buffer, shared_ptr<NFunction>& n)
{
    vector<preToken> l = makeParam(buffer, n->fContainer, n->paramMap);
    int fS;
    for(auto sksk : l){
        cout << "new Thing: " << sksk.stringValue << endl;
    }
    while (n->fContainer[fS].stringValue != "{" && fS < n->fContainer.size())
    {
        fS++;
    }
    vector<preToken> c =  n->fContainer;
    c.insert(c.begin()+fS+1, l.begin(),l.end());
    Second p(n->scope, c, backEnd);
    p.secondPass();
    if (n->returnType != VarType::Void)
    {
        if (n->scope->Cvars["return"])
        {
            valuePasser = n->scope->Cvars["return"]->value;
        }
    }
    n->scope->Cvars.clear();
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
        functionHandling(buffer, activeFunction);
        i++;
        logic.collecting = false;
        logic.runFunction = false;
        buffer.clear();
    }
    if (token.stringValue == ";" && logic.returningVlaue == true)
    {
        Variable p;
        p.type = VarType::NONE;
        getNodeTree(p ,buffer, "return");
        valuePasser = scope->Cvars["return"]->value;
        cout << " Return value: ";
         valuePasser->print();
         cout << endl;
       
        logic.collecting = false;
    }
}