#include <unordered_map>
#include "maps.h"


const std::unordered_map<std::string, TokenCategory> broadTokenMap = {
    // Keywords
    {"if", TokenCategory::KEYWORD},
    {"else", TokenCategory::KEYWORD}, 
    {"while", TokenCategory::KEYWORD},
    {"return", TokenCategory::KEYWORD},
    {"#include", TokenCategory::KEYWORD},

    {"bool",   TokenCategory::IDENTIFIER},
    {"char",   TokenCategory::IDENTIFIER},
    {"int",    TokenCategory::IDENTIFIER},
    {"float",  TokenCategory::IDENTIFIER},
    {"double", TokenCategory::IDENTIFIER},
    {"string", TokenCategory::IDENTIFIER},

    {"const", TokenCategory::PREFIX},
    {"static", TokenCategory::PREFIX},
    {"unsigned", TokenCategory::PREFIX},
    {"signed", TokenCategory::PREFIX},

    // Operators
    {"+", TokenCategory::OPERATOR},
    {"-", TokenCategory::OPERATOR},
    {"*", TokenCategory::OPERATOR},
    {"<=", TokenCategory::OPERATOR},
    {">=", TokenCategory::OPERATOR},
    {"/", TokenCategory::OPERATOR},
    {"=", TokenCategory::OPERATOR},
    {"==", TokenCategory::OPERATOR},
    {"%", TokenCategory::OPERATOR},
    {"!=", TokenCategory::OPERATOR},
    {"<", TokenCategory::OPERATOR},
    {">", TokenCategory::OPERATOR},
    {"|", TokenCategory::OPERATOR},
    {"&", TokenCategory::OPERATOR},

    // Delimiters
    {"(", TokenCategory::DELIMITER},
    {")", TokenCategory::DELIMITER},
    {"{", TokenCategory::DELIMITER},
    {"}", TokenCategory::DELIMITER},
    {",", TokenCategory::DELIMITER},
    {";", TokenCategory::DELIMITER},
    {"\"", TokenCategory::DELIMITER},

    {"&&", TokenCategory::BOOLOP},
    {"||", TokenCategory::BOOLOP},
};

 const std::unordered_map<std::string, VarType> identifierMap = {
    {"bool",   VarType::Bool},
    {"char",   VarType::Char},
    {"int",    VarType::Int},
    {"float",  VarType::Float},
    {"string", VarType::String},
    {"void", VarType::Void},
};

 const std::unordered_map<std::string, Prefix> prefixMap = {
    {"const", Prefix::Const},
    {"static", Prefix::Static},
    {"unsigned", Prefix::Unsigned},
    {"signed", Prefix::Signed}
};

const std::unordered_map<std::string, Delimiter> delimiterMap = {
    {"(", Delimiter::ParenthesisOpen},
    {")", Delimiter::ParenthesisClose},
    {"{", Delimiter::BraceOpen},
    {"}", Delimiter::BraceClose},
    {",", Delimiter::Comma},
    {";", Delimiter::Semicolon},
    {"VOID", Delimiter::NONE}
};

const std::unordered_map<std::string, OperatorType> operatorMap = {
    {"+", OperatorType::Add},
    {"-", OperatorType::Subtract},
    {"*", OperatorType::Multiply},
    {"/", OperatorType::Divide},
    {"%", OperatorType::Modulus},
    {"=", OperatorType::Assign},
};

const std::unordered_map<char, std::string> arm64Ops = {
    {'+',  "add"},      // integer addition
    {'-',  "sub"},      // integer subtraction
    {'*',  "mul"},      // integer multiplication
    {'/',  "sdiv"},     // signed integer division
    {'%',  "srem"},     // signed integer remainder (modulus)
    {'=',  "mov"}       // assignment → move value to register
};

const std::unordered_map<std::string, std::string> arm64BoolOpsInverted = {
    {"<",  "ge"},   // inverted: branch if not less than
    {"<=", "gt"},   // inverted: branch if not less than or equal
    {">",  "le"},   // inverted: branch if not greater than
    {">=", "lt"},   // inverted: branch if not greater than or equal
    {"==", "ne"},   // inverted: branch if not equal
    {"!=", "eq"}    // inverted: branch if not not equal
}; 

const std::unordered_map<std::string, std::string> arm64BoolOps = {
    {"<",  "lt"},   // branch if less than
    {"<=", "le"},   // branch if less than or equal
    {">",  "gt"},   // branch if greater than
    {">=", "ge"},   // branch if greater than or equal
    {"==", "eq"},   // branch if equal
    {"!=", "ne"}    // branch if not equal
};