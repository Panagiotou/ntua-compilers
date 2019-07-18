%{
  #define T_eof           0
  #define T_and           1001
  #define T_array         1002
  #define T_begin         1003
  #define T_boolean       1004
  #define T_char          1005
  #define T_dispose       1006
  #define T_div           1007
  #define T_do            1008
  #define T_else          1009
  #define T_end           1010
  #define T_false         1011
  #define T_forward       1012
  #define T_function      1013
  #define T_goto          1014
  #define T_if            1015
  #define T_integer       1016
  #define T_label         1017
  #define T_mod           1018
  #define T_new           1019
  #define T_nil           1020
  #define T_not           1021
  #define T_of            1022
  #define T_or            1023
  #define T_procedure     1024
  #define T_program       1025
  #define T_real          1026
  #define T_result        1027
  #define T_return        1028
  #define T_then          1029
  #define T_true          1030
  #define T_var           1031
  #define T_while         1032
  #define T_id            1033

  void ERROR (const char msg []);
%}
L [A-DF-Za-df-z]
E [Ee]
D [0-9]
W [ \t\n\r\0\\\'\"]
/* printable characters */
PR [ !#-&\(-\[\]-~]

%%
"and"             {return T_and;}
"array"           {return T_array;}
"begin"           {return T_begin;}
"boolean"         {return T_boolean;}
"char"            {return T_char;}
"dispose"         {return T_dispose;}
"div"             {return T_div;}
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
"mod"             {return T_mod;}
"new"             {return T_new;}
"nil"             {return T_nil;}
"not"             {return T_not;}
"of"              {return T_of;}
"or"              {return T_or;}
"procedure"       {return T_procedure;}
"program"         {return T_program;}
"real"            {return T_real;}
"result"          {return T_result;}
"return"          {return T_return;}
"then"            {return T_then;}
"true"            {return T_true;}
"var"             {return T_var;}
"while"           {return T_while;}

({L}|{E})({L}|{E}|{D}|"_")*               { return T_id; }
{D}+				                              {return T_int_const;}
({D}+("."{D}*({E}("+"|"-")?{D}+)?)?)      {return T_real_const;}
"'"({PR}|{W}|{L}|{E})"'"                  {return T_const_char;}
"""({PR}|{W}|{L}|{Ε})*"""                 {return T_const_string;}

"="                                {return T_op_eq;}
">"                                {return T_op_g;}
"<"                                {return T_op_l;}
"<>"                               {return T_op_neq;}
"<="                               {return T_op_leq;}
">="                               {return T_op_geq;}
"+"                                {return T_op_p;}
"-"                                {return T_op_m;}
"*"                                {return T_op_mul;}
"/"                                {return T_op_d;}
"^"                                {return T_op_point;}
"@"                                {return T_op_addr;}

":="                               {return T_op_assign;}
";"                                {return T_op_semicol;}
"."                                {return T_op_dot;}
"("                                {return T_op_lpar;}
")"                                {return T_op_rpar;}
":"                                {return T_op_col;}
","                                {return T_op_com;}
"["                                {return T_op_lbr;}
"]"                                {return T_op_rbr;}

{W}+                               {/* nothing */}
"(*"([^*]+[\*+[^*)])*\*")"         {/* nothing */}
.                                  {ERROR("illegal token");}
%%
int main () {
  int token;
  do {
    token = yylex();
    printf("token=%d, lexeme=\"%s\"\n", token, yytext);
  } while (token != T_eof);
}

void ERROR (const char msg [])
{
  fprintf(stderr, "ERROR: %s\n", msg);
  exit(1);
}
