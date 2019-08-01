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
  Body *body;
  Local *local;
  Block *block;
  Header *header;
  Label *label;
  Id_list *id_list;
  Decl *decl;
  Decl_list *decl_list;


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

%type<body>  body local_list
%type<block>  block
%type<local>  local
%type<header>  header
%type<label>  decl_label
%type<id_list> id_list
%type<decl_list> decl_list
%type<decl> decl

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
  "program" T_id ";" body "."{
    $4->sem();
    // std::cout << "AST: " << *$1 << std::endl;
    //$1->run();
  }
  ;

body:
  /*nothing*/ { $$ = new Body(); }
  |local_list block { $1->merge($2); $$ = $1; }
  ;

local_list:
  /*nothing*/ { $$ = new Body(); }
  |local_list local { $1->append_local($2); $$ = $1; }
  ;

local:
 "var" decl_list { $$ = new Local($2); }
 | "label" decl_label { $$ = new Local($2); }
 | header ";" body ";" { $$ = new Local($1, $3); }
 | "forward" header ";" { $$ = new Local($2); }
 ;

decl_label:
  T_id id_list ";" { $2->append_id($1); $$ = new Label($2); }
  ;


id_list:
  /*nothing*/ { $$ = new Id_list(); }
  | id_list "," T_id { $1->append_id($3); $$ = $1; }
  ;

decl_list:
  decl { $$ = new Decl_list(); $$->append_decl($1);}
  | decl_list decl { $1->append_decl($2); $$ = $1; }
  ;

decl:
  T_id id_list ":" type ";" { $2->append_id($1); $$ = new Decl($2, $4);}

header:
 "procedure" T_id "(" formal formal_list  ")"
 | "procedure" T_id "(" ")"
 | "function" T_id "(" formal formal_list  ")" ":" type
 | "function" T_id "(" ")" ":" type
 ;

formal_list:
  /*nothing*/
  | formal_list ";" formal
  ;

formal:
  "var" T_id  id_list ":" type
  |T_id  id_list ":" type
  ;

type:
 "integer"
 | "real"
 | "boolean"
 | "char"
 | "array" "[" T_int_const "]" "of" type
 | "array" "of" type
 | "^" type
 ;

block:
  "begin" stmt stmt_list "end"
  ;

stmt_list:
  /*nothing*/
  | stmt_list ";" stmt
  ;

stmt:
  /*nothing*/
  | l-value ":=" expr
  | expr "^" ":=" expr
  | block
  | call
  | "if" expr "then" stmt
  | "if" expr "then" stmt "else" stmt
  | "while" expr "do" stmt
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
 T_id
 | "result"
 | T_const_string
 | l-value "[" expr "]"
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
 | "not" expr
 | "+" expr
 | "-" expr
 | expr "+" expr
 | expr "-" expr
 | expr "*" expr
 | expr "/" expr
 | expr "div" expr
 | expr "mod" expr
 | expr "or" expr
 | expr "and" expr
 | expr "=" expr
 | expr "<>" expr
 | expr "<" expr
 | expr "<=" expr
 | expr ">" expr
 | expr ">=" expr
 ;

call:
  T_id "(" expr  expr_list  ")"
  |T_id "(" ")"
  ;

expr_list:
  /*nothing*/
  | expr_list "," expr
  ;



%%

int main() {
  int result = yyparse();
  if (result == 0) printf("Success.\n");
  return result;
}
