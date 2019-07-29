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

%token T_op_eq "="
%token T_op_g ">"
%token T_op_l "<"
%token T_op_neq "<>"
%token T_op_leq "<="
%token T_op_geq ">="
%token T_op_p "+"
%token T_op_m "-"
%token T_op_mul "*"
%token T_op_d "/"
%token T_op_point "^"
%token T_op_addr "@"

%token T_op_assign ":="
%token T_op_semicol ";"
%token T_op_dot "."
%token T_op_lpar "("
%token T_op_rpar ")"
%token T_op_col ":"
%token T_op_com ","
%token T_op_lbr "["
%token T_op_rbr "]"

%token T_int_const
%token T_real_const
%token T_const_char
%token T_const_string
%token<op> T_id

/*operators*/
%left<op> "+" "-" "="
%left<op> "*" "div" "mod" "or"
%left<op> "(" ")" "[" "]" "<=" ">=" "<" ">" "<>" "and" "/" "not"
%right<op>  "@" "^"

%expect 1

%union {
  Stmt *stmt;
  Expr *expr;
  Type type;
  Rval *rval;
  Lval *lval;
  int num;
  double re;
  char var;
  char *op;
}

%type<stmt>  stmt
%type<expr>  expr
%type<type>  type
%type<rval>  r-value
%type<lval>  l-value
%type<num>    T_int_const
%type<var>    T_const_char
%type<re>    T_real_const
%type<op>    T_const_string

%%

program:
  "program" T_id ";" body "." {
    $2->sem();
    $4->sem();
    std::cout << "AST: " << *$4 << std::endl;
  }
  ;

body:
  /*nothing*/
  |local_list block
  ;

local_list:
  /*nothing*/
  |local_list local
  ;

local:
 "var" decl_list
 | "label" decl_label
 | header ";" body ";"
 | "forward" header ";"
 ;

decl_label:
  T_id id_list ";"
  ;


id_list:
  /*nothing*/
  | id_list "," T_id
  ;

decl_list:
  decl
  | decl_list decl
  ;

decl:
  T_id id_list ":" type ";"

header:
 "procedure" T_id "(" formal  header1  ")"
 | "procedure" T_id "(" ")"
 | "function" T_id "(" formal  ";" formal  ")" ":" type
 | "function" T_id "(" ")" ":" type
 ;

header1:
  /*nothing*/
  | header1 ";" formal
  ;

formal:
  "var" T_id  local1 ":" type
  |T_id  local1 ":" type
  ;

type:
 "integer"                                  { $$ = TYPE_INTEGER; }
 | "real"                                   { $$ = TYPE_REAL; }
 | "boolean"                                { $$ = TYPE_BOOLEAN; }
 | "char"                                   { $$ = TYPE_CHAR; }
 | "array" "[" T_int_const "]" "of" type
 | "array" "of" type
 | "^" type
 ;

block:
  "begin" stmt block1 "end"
  ;

block1:
  /*nothing*/
  | block1 ";" stmt
  ;

stmt:
  /*nothing*/
  | l-value ":=" expr
  | expr "^" ":=" expr
  | block
  | call
  | "if" expr "then" stmt {std::cout<<$2; $$ = new If($2, $4); }
  | "if" expr "then" stmt "else" stmt { std::cout<<$2; $$ = new If($2, $4, $6); }
  | "while" expr "do" stmt { std::cout<<$2; $$ = new While($2, $4); }
  | T_id ":" stmt
  | "goto" T_id
  | "return"
  | "new" "[" expr "]" l-value
  | "new" "[" expr "]" expr "^"
  | "new" l-value
  | "new" expr "^"
  | "dispose" "[" "]" l-value
  | "dispose" "[" "]" expr "^"
  | "dispose" l-value
  | "dispose" expr "^"
  ;

expr:
 l-value
 | r-value
 ;

l-value:
 T_id { std::cout<<$1; $$ = new Id($1); }
 | "result"
 | T_const_string { std::cout<<$1; $$ = new Conststring($1); }
 | l-value "[" expr "]"
 | "(" l-value ")"
 ;


r-value:
 T_int_const { std::cout<<$1; $$ = new Constint($1); }
 | "true"
 | "false"
 | T_real_const { std::cout<<$1; $$ = new Constreal($1); }
 | T_const_char { std::cout<<$1; $$ = new Constchar($1); }
 | "(" r-value ")"
 | "nil"
 | call
 | "@" l-value
 | "not" expr { std::cout<<$1; $$ = new UnOp($1, $2); }
 | "+" expr { std::cout<<$1; $$ = new UnOp($1, $2); }
 | "-" expr { std::cout<<$1; $$ = new UnOp($1, $2); }
 | expr "+" expr { std::cout<<$1; $$ = new BinOp($1, $2, $3); }
 | expr "-" expr { std::cout<<$1; $$ = new BinOp($1, $2, $3); }
 | expr "*" expr { std::cout<<$1; $$ = new BinOp($1, $2, $3); }
 | expr "/" expr { std::cout<<$1; $$ = new BinOp($1, $2, $3); }
 | expr "div" expr { std::cout<<$1; $$ = new BinOp($1, $2, $3); }
 | expr "mod" expr { std::cout<<$1; $$ = new BinOp($1, $2, $3); }
 | expr "or" expr { std::cout<<$1; $$ = new BinOp($1, $2, $3); }
 | expr "and" expr { std::cout<<$1; $$ = new BinOp($1, $2, $3); }
 | expr "=" expr { std::cout<<$1; $$ = new BinOp($1, $2, $3); }
 | expr "<>" expr { std::cout<<$1; $$ = new BinOp($1, $2, $3); }
 | expr "<" expr { std::cout<<$1; $$ = new BinOp($1, $2, $3); }
 | expr "<=" expr { std::cout<<$1; $$ = new BinOp($1, $2, $3); }
 | expr ">" expr { std::cout<<$1; $$ = new BinOp($1, $2, $3); }
 | expr ">=" expr { std::cout<<$1; $$ = new BinOp($1, $2, $3); }
 ;

call:
  T_id "(" expr  call1  ")"
  |T_id "(" ")"
  ;

call1:
  /*nothing*/
  | call1 "," expr
  ;



%%

int main() {
  int result = yyparse();
  if (result == 0) printf("Success.\n");
  return result;
}
