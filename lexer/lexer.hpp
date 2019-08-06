#ifndef __LEXER_HPP__
#define __LEXER_HPP__
#include <vector>

extern std::vector<char *> ids;
extern std::vector<double> doubles;
extern std::vector<int> ints;
extern std::vector<char> chars;
extern std::vector<std::string> strings;

int yylex();
void yyerror(const char *msg);

#endif
