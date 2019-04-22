%option noyywrap

%{
#include <cstdio>
#include <cstdlib>
#include "lexer.hpp"
#include "parser.hpp"
%}

L [a-z]
D [0-9]
W [ \t\r\n]

%%

"for"   { return T_for; }
"print" { return T_print; }
"let"   { return T_let; }
"if"    { return T_if; }
"then"  { return T_then; }
"else"  { return T_else; }
"do"    { return T_do; }
"begin" { return T_begin; }
"end"   { return T_end; }

{L}     { return T_id; }
{D}+    { return T_const; }

[()+\-/%*=]   { return yytext[0]; }

{W}+    { /* nothing */ }
\'.*\n  { /* nothing' */ }

.       { yyerror("lexical error"); }

%%

void yyerror(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
}
