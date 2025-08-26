#include "reading.h"
#include "maps/maps.h"
#include <unordered_set>
#include <fstream>
#include <string>
#include <iostream>
#include <unistd.h>

bool isBreakingChar(char c) {
    static const std::unordered_set<char> breakingChars = {
        ' ', '\t', '\n', '\r', ';', ',', '.', '(', ')', '{', '}', '[', ']',
        '+', '-', '*', '/', '=', '<', '>', '!', '&', '|', '^', '%', ':', '"'
    };

    return breakingChars.count(c) > 0;
}

void reading::setMemory(string path){
    std::ifstream file;
    file.open(path, std::ios::binary);
    if (!file.is_open())
    {
        std::cout << "Error opening file";
        int a;
        std::cin >> a;

    }
    mainMemory.assign((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
}

void reading::reader(){
    preToken token;
    bool collect = false;
    for (int i = 0; i < mainMemory.size(); i++)
    {
        if (mainMemory[i] == '"')
        {
            collect = !collect;
            token.stringValue += '"';
            continue;
        }
        
        if ((isBreakingChar(mainMemory[i]) && collect == false))
        {
            auto it = broadTokenMap.find(token.stringValue);
            if(it != broadTokenMap.end()){
                token.cat = it->second;
                preTokens.push_back(token);
                token.stringValue.clear();
            }
            else{
                if(all_of(token.stringValue.begin(),token.stringValue.end(), ::isdigit) && token.stringValue != ""){
                    token.cat = TokenCategory::NUMBER;
                    preTokens.push_back(token);
                    token.stringValue.clear();
                }
                else if(token.stringValue[0] == '"' && token.stringValue.back() == '"')
                {
                    token.cat = TokenCategory::STRING;
                    token.stringValue.erase(0,1);
                    token.stringValue.erase(token.stringValue.size()-1);
                    preTokens.push_back(token);
                    token.stringValue.clear();
                }
                else if (token.stringValue != "")
                {
                    token.cat = TokenCategory::VARIABLE;
                    preTokens.push_back(token);
                    token.stringValue.clear();
                }
                
                
            }
            std::string d(1,mainMemory[i]);
            it = broadTokenMap.find(d);
            if (it != broadTokenMap.end())
            {
                token.cat = it->second;
                token.stringValue = d;
                preTokens.push_back(token);
            }
            token.stringValue.clear();
        }
        else{
            token.stringValue += mainMemory[i];
        }
    }
}

void reading::firstPass(){
    bool inFunction = false;
    bool inParams = false;
    int track = 0;
    int track2 = 0;
    std::vector<preToken> buffer;
    for (int i = 0; i < preTokens.size(); i++)
    {
        if (preTokens[i].cat == TokenCategory::DELIMITER)
        {
          auto it = delimiterMap.find(preTokens[i].stringValue);
          if (it != delimiterMap.end())
          {
            Delimiter del = it->second;
            switch (del)
            {
                case Delimiter::ParenthesisOpen:{
                    if (nFunction == nullptr)
                    { 
                        if (!inFunction)
                        {
                            track = i;
                        }
                        inParams = true; 
                        nFunction = std::make_shared<NFunction>();
                        nFunction->parentScope = scope;
                        nFunction->scope = make_shared<Scope>();
                        nFunction->scope->functionMap = scope->functionMap;
                        functionAssigning(nFunction, buffer);
                        buffer.clear();
                    }
                    break;
                }
                case Delimiter::ParenthesisClose:{
                    if (inParams == true)
                    {
                        if (buffer.size() > 0)
                        {
                            preToken pToken;
                            pToken.stringValue = ";";
                            pToken.cat = TokenCategory::DELIMITER;
                            buffer.push_back(pToken);
                        }
                        order.clear();
                        variableAssigning(nFunction->paramMap, scope->functionMap, buffer);
                        nFunction->scope->variableMap = nFunction->paramMap;
                        nFunction->paramOrder = order;
                        inParams = false;
                        buffer.clear();
                    }
                    break;
                }
                case Delimiter::Semicolon:{
                    if (!inFunction)
                    {
                        track2 = i;
                    }
                    buffer.push_back(preTokens[i]);
                    variableAssigning(scope->variableMap, scope->functionMap, buffer);
                    buffer.clear();
                    break;
                }
                case Delimiter::BraceOpen:{
                    inFunction = true;
                    scope = nFunction->scope;
                    break;
                }
                case Delimiter::BraceClose:{
                    cout << "inserted function: \"" << nFunction->name << "\"" << endl;
                    scope = nFunction->parentScope;
                    scope->functionMap.emplace(std::make_pair(nFunction->name,std::move(nFunction)));
                    nFunction = nullptr;
                    inFunction = false;
                    break;
                }
                default:
                    break;
            }
        }
        else{
            cout << "error1";
        }
            
        }
        else{
            buffer.push_back(preTokens[i]);
        }        
    }
}

void reading::functionAssigning(shared_ptr<NFunction> nFunction, std::vector<preToken>& buffer){
    for (int i = 0; i < buffer.size(); i++)
    {
        switch (buffer[i].cat)
        {
        case TokenCategory::IDENTIFIER:{
            auto it = identifierMap.find(buffer[i].stringValue);
            nFunction->returnType = it->second;
            break;
        }
         case TokenCategory::VARIABLE:{
            nFunction->name = buffer[i].stringValue;
            break;
        }
        case TokenCategory::PREFIX:{
            auto it = prefixMap.find(buffer[i].stringValue);
            nFunction->prefix = it->second;
            break;
        }
        default:
            break;
        }
    }
}

void reading::variableAssigning(std::unordered_map<std::string, Variable>& variableMap, std::unordered_map<std::string,shared_ptr<NFunction>>& functionMap, std::vector<preToken>& buffer){
    Variable var;
    bool breaker = false;
    std::string name;
    for (int i = 0; i < buffer.size(); i++)
    {
        auto& token = buffer[i];
        switch (token.cat)
        {
        case TokenCategory::PREFIX:{
            auto pre = prefixMap.find(token.stringValue);
            if(pre != prefixMap.end())
            {
                var.prefixes.push_back(pre->second);
            }
            break;
        }
        case TokenCategory::IDENTIFIER:{
            auto id = identifierMap.find(token.stringValue);
            if(id != identifierMap.end())
            {
                var.type = id->second;
            }
            break;
        }
        case TokenCategory::VARIABLE:{
            if (functionMap.find(token.stringValue) == functionMap.end() && variableMap.find(token.stringValue) == variableMap.end())
            {
                name = token.stringValue;
            }
            break;
        }
        case TokenCategory::DELIMITER:{
            if (token.stringValue == ";")
            {
                variableMap.insert(make_pair(name, var));
                order.push_back(name);
            }
            break;
        }
         case TokenCategory::OPERATOR:{
            if (token.stringValue == "=")
            {
                variableMap.insert(make_pair(name, var));
                breaker = true;
            }
            break;
        }
        default:
            break;
        }
        if(breaker)
        {
            break;
        }
    }
}

