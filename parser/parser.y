%{
  #include <cstdio>
  #include "../lexer/lexer.hpp"
%}
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


%token T_op_eq          "="
%token T_op_g           ">"
%token T_op_l           "<"
%token T_op_neq         "<>"
%token T_op_leq         "<="
%token T_op_geq         ">="
%token T_op_p           "+"
%token T_op_m           "-"
%token T_op_mul         "*"
%token T_op_d           "/"
%token T_op_point       "^"
%token T_op_addr        "@"

%token T_op_assign      ":="
%token T_op_semicol     ";"
%token T_op_dot         "."
%token T_op_lpar        "("
%token T_op_rpar        ")"
%token T_op_col         ":"
%token T_op_com         ","
%token T_op_lbr         "["
%token T_op_rbr         "]"
%token T_int_const
%token T_real_const
%token T_const_char
%token T_const_string
%token T_id

/*operators*/
%left "or"
%left "and"
%left "not"
%nonassoc '=' "<>" '>' '<' "<=" ">="
%left '^'
%left '+' '-'
%left '*' '/' "mod" "div"

%%

program:
  "program" T_id ";" body "."
  ;

body:
  local "∗" block

local:
 "var" T_id  "," T_id "∗" ":" type ";"
 | "label" T_id  "," T_id ";"
 | header ";" body ";"
 | "forward" header ";"
 ;

header:
 "procedure" T_id "(" formal  ";" formal  ")"
 | "function" T_id "(" formal  ";" formal  ")" ":" type
 ;

formal:
  "var" T_id  "," T_id ":" type
  ;

type:
 "integer"
 | "real"
 | "boolean"
 | "char"
 | "array" "[" T_int_const "]" "of" type
 | "^" type
 ;

block:
  "begin" stmt ";" stmt "end"
  ;

stmt:
  /*nothing*/
  | l-value ":=" expr
  | block
  | call
  | "if" expr "then" stmt "else" stmt
  | "while" expr "do" stmt
  | T_id ":" stmt
  | "goto" T_id
  | "return"
  | "new" "[" expr "]" l-value
  | "dispose" "[" "]" l-value
  ;

expr:
 l-value
 | r-value
 ;

l-value:
 T_id
 | "result"
 | T_const_string
 | l-value "[" expr"]"
 | expr "^"
 | "(" l-value ")"
 ;

r-value:
 T_int_const
 | "true"
 | "false"
 | T_real_const
 | T_const_char
 | "(" r-value ")"
 | "nil"
 | call
 | "@" l-value
 | unop expr
 | expr binop expr
 ;

call:
  T_id "(" expr  "," expr  ")"
  ;

unop:
  "not"
  | "+"
  | "-"
  ;

binop:
  "+"
  | "-"
  | "*"
  | "/"
  | "div"
  | "mod"
  | "or"
  | "and"
  | "="
  | "<>"
  | "<"
  | "<="
  | ">"
  | ">="
  ;

%%

int main() {
  int result = yyparse();
  if (result == 0) printf("Success.\n");
  return result;
}
