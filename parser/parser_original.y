%{
  #include <cstdio>
  #include <string>
  #include "../semantic/ast.hpp"
  #include "../lexer/lexer.hpp"

  SymbolTable st;
  std::vector<int> rt_stack;
%}

%define parse.error verbose
%verbose
%define parse.trace


%token T_and          "and"
%token T_array        "array"
%token T_begin        "begin"
%token T_boolean      "boolean"
%token T_char         "char"
%token T_dispose      "dispose"
%token T_div          "div"
%token T_do           "do"
%token T_else         "else"
%token T_end          "end"
%token T_false        "false"
%token T_forward      "forward"
%token T_function     "function"
%token T_goto         "goto"
%token T_if           "if"
%token T_integer      "integer"
%token T_label        "label"
%token T_mod          "mod"
%token T_new          "new"
%token T_nil          "nil"
%token T_not          "not"
%token T_of           "of"
%token T_or           "or"
%token T_procedure    "procedure"
%token T_program      "program"
%token T_real         "real"
%token T_result       "result"
%token T_return       "return"
%token T_then         "then"
%token T_true         "true"
%token T_var          "var"
%token T_while        "while"

%token T_op_point       '^'
%token T_op_addr        '@'
%token T_op_assign      ":="

%token T_op_neq         "<>"
%token T_op_leq         "<="
%token T_op_geq         ">="

%token T_int_const
%token T_real_const
%token T_const_char
%token T_const_string
%token T_id

/*operators*/
%left '+' '-' '='
%left '*' "div" "mod" "or"
%left '(' ')' '[' ']' "<=" ">=" '<' '>' "<>" "and" '/' "not"
%right '@' '^'

%expect 1


%%

program:
  "program" T_id ';' body '.'
  ;

body:
  /*nothing*/
  |localst block
  ;

localst:
  /*nothing*/
  |localst local
  ;

local:
 "var" local2
 | "label" T_id local1 ';'
 | header ';' body ';'
 | "forward" header ';'
 ;
local1:
  /*nothing*/
  | local1 ',' T_id
  ;

local2:
  T_id local1 ':' type ';'
  | local2 T_id local1 ':' type ';'
  ;

header:
 "procedure" T_id '(' formal  header1  ')'
 | "procedure" T_id '(' ')'
 | "function" T_id '(' formal  ';' formal  ')' ':' type
 | "function" T_id '(' ')' ':' type
 ;

header1:
  /*nothing*/
  | header1 ';' formal
  ;

formal:
  "var" T_id  local1 ':' type
  |T_id  local1 ':' type
  ;

type:
 "integer"
 | "real"
 | "boolean"
 | "char"
 | "array" '[' T_int_const ']' "of" type
 | "array" "of" type
 | '^' type
 ;

block:
  "begin" stmt block1 "end"
  ;

block1:
  /*nothing*/
  | block1 ';' stmt
  ;

stmt:
  /*nothing*/
  | l-value ":=" expr
  | expr '^' ":=" expr
  | block
  | call
  | "if" expr "then" stmt
  | "if" expr "then" stmt "else" stmt
  | "while" expr "do" stmt
  | T_id ':' stmt
  | "goto" T_id
  | "return"
  | "new" '[' expr ']' l-value
  | "new" '[' expr ']' expr '^'
  | "new" l-value
  | "new" expr '^'
  | "dispose" '[' ']' l-value
  | "dispose" '[' ']' expr '^'
  | "dispose" l-value
  | "dispose" expr '^'
  ;

expr:
 l-value
 | r-value
 ;

l-value:
 T_id
 | "result"
 | T_const_string
 | l-value '[' expr ']'
 | '(' l-value ')'
 ;


r-value:
 T_int_const
 | "true"
 | "false"
 | T_real_const
 | T_const_char
 | '(' r-value ')'
 | "nil"
 | call
 | '@' l-value
 | "not" expr
 | '+' expr
 | '-' expr
 | expr '+' expr
 | expr '-' expr
 | expr '*' expr
 | expr '/' expr
 | expr "div" expr
 | expr "mod" expr
 | expr "or" expr
 | expr "and" expr
 | expr '=' expr
 | expr "<>" expr
 | expr '<' expr
 | expr "<=" expr
 | expr '>' expr
 | expr ">=" expr
 ;

call:
  T_id '(' expr  call1  ')'
  |T_id '(' ')'
  ;

call1:
  /*nothing*/
  | call1 ',' expr
  ;



%%

int main() {
  int result = yyparse();
  if (result == 0) printf("Success.\n");
  return result;
}
