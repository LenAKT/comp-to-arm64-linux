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

const std::unordered_map<int, std::string> arm64LoadOps = {
    {1, "ldrb"},   // load byte
    {2, "ldrh"},   // load halfword (16-bit)
    {4, "ldr"},    // load word (32-bit)
    {8, "ldr"},    // load doubleword (64-bit)
    {16, "ldp"}    // load pair of registers (8+8 bytes)
};

// Store instructions (size in bytes -> mnemonic)
const std::unordered_map<int, std::string> arm64StoreOps = {
    {1, "strb"},   // store byte
    {2, "strh"},   // store halfword
    {4, "str"},    // store word (32-bit)
    {8, "str"},    // store doubleword (64-bit)
    {16, "stp"}    // store pair of registers (8+8 bytes)
};