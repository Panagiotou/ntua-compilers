%{
#include <cstdio>
#include "lexer.hpp"
%}

%token T_for "for"
%token T_print "print"
%token T_let "let"
%token T_if "if"
%token T_then "then"
%token T_else "else"
%token T_do "do"
%token T_begin "begin"
%token T_end "end"
%token T_id
%token T_const

%left '+' '-'
%left '*' '/' '%'

%expect 1

%%

program:
  stmt_list
;

stmt_list:
  /* nothing */
| stmt_list stmt
;

stmt:
  "let" T_id '=' expr
| "print" expr
| "for" expr "do" stmt
| "if" expr "then" stmt
| "if" expr "then" stmt "else" stmt
| "begin" stmt_list "end"
;

expr:
  T_id
| T_const
| '(' expr ')'
| expr '+' expr
| expr '-' expr
| expr '*' expr
| expr '/' expr
| expr '%' expr
;

%%

int main() {
  int result = yyparse();
  if (result == 0) printf("Success.\n");
  return result;
}
