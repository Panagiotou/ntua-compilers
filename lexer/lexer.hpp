#ifndef __LEXER_HPP__
#define __LEXER_HPP__
#include <vector>

extern std::vector<char *> ids;
extern std::vector<int> constInts;
extern std::vector<double> constReals;
extern std::vector<char> constChars;
extern std::vector<char *> constStrings;
extern std::vector<char *> operators;

int yylex();
void yyerror(const char *msg);

#endif
