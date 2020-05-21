%{
  #include <cstdio>
  #include <cstdlib>
  #include <string>
  #include <vector>
  #include "../semantic/ast.hpp"
  #include "lexer.hpp"
  #include "../parser/parser.hpp"
  #define T_eof  0

  std::vector<char *> ids;

%}
L [A-DF-Za-df-z]
E [Ee]
D [0-9]
/* escape sequence */
Esc \\[ "t""n""r""0"\'\"\\]

%x COMMENT
%option noyywrap
%option nounput




%%
"and"             { yylval.op = strdup(yytext); return T_and;}
"array"           {return T_array;}
"begin"           {return T_begin;}
"boolean"         {return T_boolean;}
"char"            {return T_char;}
"dispose"         {return T_dispose;}
"div"             { yylval.op = strdup(yytext); return T_div;}
"do"              {return T_do;}
"else"            {return T_else;}
"end"             {return T_end;}
"false"           {return T_false;}
"forward"         {return T_forward;}
"function"        {return T_function;}
"goto"            {return T_goto;}
"if"              {return T_if;}
"integer"         {return T_integer;}
"label"           {return T_label;}
"mod"             { yylval.op = strdup(yytext); return T_mod;}
"new"             {return T_new;}
"nil"             {return T_nil;}
"not"             { yylval.op = strdup(yytext); return T_not;}
"of"              {return T_of;}
"or"              { yylval.op = strdup(yytext); return T_or;}
"procedure"       {return T_procedure;}
"program"         {return T_program;}
"real"            {return T_real;}
"result"          {return T_result;}
"return"          {return T_return;}
"then"            {return T_then;}
"true"            {return T_true;}
"var"             {return T_var;}
"while"           {return T_while;}

"=" { yylval.op = strdup(yytext); return T_op_eq;}
">" { yylval.op = strdup(yytext); return T_op_g;}
"<" {yylval.op = strdup(yytext); return T_op_l;}
"<>" { yylval.op = strdup(yytext); return T_op_neq;}
"<=" { yylval.op = strdup(yytext); return T_op_leq;}
">=" { yylval.op = strdup(yytext); return T_op_geq;}
"+" { yylval.op = strdup(yytext); return T_op_p;}
"-" { yylval.op = strdup(yytext); return T_op_m;}
"*" { yylval.op = strdup(yytext); return T_op_mul;}
"/" { yylval.op = strdup(yytext); return T_op_d;}
"^" {return T_op_point;}
"@" {return T_op_addr;}

":=" {return T_op_assign;}
";" {return T_op_semicol;}
"." {return T_op_dot;}
"(" {return T_op_lpar;}
")" {return T_op_rpar;}
":" {return T_op_col;}
"," {return T_op_com;}
"[" {return T_op_lbr;}
"]" {return T_op_rbr;}

({L}|{E})({L}|{E}|{D}|"_")*               {yylval.ids = strdup(yytext); return T_id; }
{D}+				                              {yylval.num = std::stoi(yytext); return T_int_const;}
({D}+("."{D}*({E}("+"|"-")?{D}+)?)?)      {yylval.re = std::stod(yytext); return T_real_const;}
\'(({Esc})|[^\"\'\\])\'                   {yylval.ch = strdup(yytext); return T_const_char;}        /*'*/
\"([^\'\"\r\n\\]|({Esc}))*\"              {yylval.stri = strdup(yytext); return T_const_string;}      /*"*/


[()+\-/%*=^\[\];:!,<>\.]          { return yytext[0]; }





[ \t\r\n]+                           {/* nothing */}


"(*" { BEGIN (COMMENT); }
<COMMENT>"*)" { BEGIN (INITIAL); }
<COMMENT>\n { /* new line */ }
<COMMENT>"*" { /* nothing */ }
<COMMENT>[^*\n]+ { /* nothing */ }


.                                  {yyerror("lexical error");}
%%

/* int main () {
  int token;
  do {
    token = yylex();
    printf("token=%d, lexeme=\"%s\"\n", token, yytext);
  } while (token != T_eof);
} */

void yyerror(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
}
