%{
  #include <cstdio>
  #include <string.h>
  #include "../semantic/ast.hpp"
  #include "../lexer/lexer.hpp"

  SymbolTable st;
  std::vector<int> rt_stack;
  #define DEBUGPARSER true

  LLVMContext AST::TheContext;
  IRBuilder<> AST::Builder(TheContext);
  std::unique_ptr<Module> AST::TheModule;
  std::unique_ptr<legacy::FunctionPassManager> AST::TheFPM;

  GlobalVariable *AST::TheVars;
  GlobalVariable *AST::TheNL;
  Function *AST::TheWriteInteger;
  Function *AST::TheWriteString;

  Type *AST::i8 = IntegerType::get(TheContext, 8);
  Type *AST::i32 = IntegerType::get(TheContext, 32);
  Type *AST::i64 = IntegerType::get(TheContext, 64);

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

%token<num> T_int_const
%token<re> T_real_const
%token<var> T_const_char
%token<sval> T_const_string
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
  Local_list *local_list;
  Block *block;
  Header *header;
  Label *label;
  Id_list *id_list;
  Decl *decl;
  Decl_list *decl_list;
  Formal *formal;
  Formal_list *formal_list;
  Stmt *stmt;
  Stmt_list *stmt_list;
  Expr *expr;
  Expr_list *expr_list;
  Call *call;
  Callr *callr;

  OurType *type;
  int num;
  double re;
  char var;
  char *op;

  std::string* sval;
}

%type<body>  body
%type<block>  block
%type<local>  local
%type<local_list>  local_list
%type<header>  header
%type<label>  decl_label
%type<id_list> id_list
%type<decl_list> decl_list
%type<decl> decl
%type<formal_list> formal_list
%type<formal> formal
%type<stmt>  stmt
%type<stmt_list> stmt_list
%type<expr_list> expr_list
%type<call> call
%type<callr> callr


%type<expr>  expr l-value r-value
%type<type>  type


%%

program:
  "program" T_id ";" body "."{
    st.openScope();
    Library *l = new Library();
    l->init(); // Initialize all built in functions and procedures
    $4->sem();
    if(DEBUGPARSER) $4->printOn(std::cout);
    
    $4->llvm_compile_and_dump();
    // std::cout << "AST: " << *$1 << std::endl;
    //$1->run();
    st.closeScope();
  }
  ;

body: local_list block { $$ = new Body($1, $2); }
  ;

local_list:
  /*nothing*/ { $$ = new Local_list(); }
  |local_list local { $1->append_local($2); $$ = $1; }
  ;

local:
 "var" decl_list { $$ = new Local($2); }
 | "label" decl_label { $$ = new Local($2); }
 | header ";" body ";" { $$ = new Local($1, $3); }
 | "forward" header ";" { $$ = new Local($2); }
 ;

decl_label:
  T_id id_list ";" { $2->append_begin(ids.back()); ids.pop_back(); $$ = new Label($2); }
  ;


id_list:
  /*nothing*/ { $$ = new Id_list(); }
  | id_list "," T_id { $1->append_id(ids.back()); ids.pop_back(); $$ = $1; }
  ;

decl_list:
  decl { $$ = new Decl_list(); $$->append_decl($1);}
  | decl_list decl { $1->append_decl($2); $$ = $1; }
  ;

decl:
  T_id id_list ":" type ";" { $2->append_begin(ids.back()); ids.pop_back();$$ = new Decl($2, $4);}

header:
 "procedure" T_id "(" formal formal_list  ")" { $5->append_begin($4); $$ = new Procedure(ids.back(), $5); ids.pop_back(); }
 | "procedure" T_id "(" ")" { $$ = new Procedure(ids.back()); ids.pop_back(); }
 | "function" T_id "(" formal formal_list  ")" ":" type { $5->append_begin($4); $$ = new OurFunction(ids.back(), $8, $5); ids.pop_back();}
 | "function" T_id "(" ")" ":" type { $$ = new OurFunction(ids.back(), $6); ids.pop_back();}
 ;

formal_list:
  /*nothing*/ { $$ = new Formal_list(); }
  | formal_list ";" formal { $1->append_formal($3); $$ = $1; }
  ;

formal:
  "var" T_id  id_list ":" type { $3->append_begin(ids.back()); ids.pop_back(); $$ = new Formal($3, $5, true); }
  |T_id  id_list ":" type { $2->append_begin(ids.back()); ids.pop_back(); $$ = new Formal($2, $4, false); }
  ;

type:
 "integer" { $$ = new Integer(); }
 | "real" { $$ = new Real(); }
 | "boolean" { $$ = new Boolean(); }
 | "char" { $$ = new Char(); }
 | "array" "[" T_int_const "]" "of" type { $$ = new Array($6, $3); }
 | "array" "of" type { $$ = new Array($3); }
 | "^" type { $$ = new Pointer($2); }
 ;

block:
  "begin" stmt stmt_list "end" { $3->append_begin($2); $$ = new Block($3); }
  ;

stmt_list:
  /*nothing*/ { $$ = new Stmt_list(); }
  | stmt_list ";" stmt { $1->append_stmt($3); $$ = $1; }
  ;

stmt:
  /*nothing*/ { $$ = nullptr;}
  | l-value ":=" expr { $$ = new Assign($1, $3); }
  | block { $$ = $1; }
  | call { $$ = $1; }
  | "if" expr "then" stmt { $$ = new If($2, $4); }
  | "if" expr "then" stmt "else" stmt { $$ = new If($2, $4, $6); }
  | "while" expr "do" stmt { $$ = new While($2, $4); }
  | T_id ":" stmt { $$ = new IdLabel(ids.back(), $3); ids.pop_back(); }
  | "goto" T_id { $$ = new Goto(ids.back()); ids.pop_back(); }
  | "return" { $$ = new Return(); }
  | "new" "[" expr "]" l-value { $$ = new New($3, $5); }
  | "new" l-value { $$ = new New($2); }
  | "dispose" "[" "]" l-value { $$ = new Dispose($4, true); }
  | "dispose" l-value { $$ = new Dispose($2, false); }
  ;

expr:
 l-value { $$ = $1; }
 | r-value { $$ = $1; }
 ;

l-value:
 T_id { $$ = new Id(ids.back()); ids.pop_back(); }
 | "result" { $$ = new Result(); }
 | T_const_string { $$ = new Conststring(constStrings.back()); constStrings.pop_back();}
 | l-value "[" expr "]" { $$ = new ArrayItem($1, $3); }
 | "(" l-value ")" { $$ = $2; }
 | expr "^" { $$ = new Dereference($1); }
 ;

r-value:
 T_int_const { $$ = new Constint(constInts.back()); constInts.pop_back(); }
 | "true" { $$ = new Constboolean("true"); }
 | "false" { $$ = new Constboolean("false"); }
 | T_real_const { $$ = new Constreal(constReals.back()); constReals.pop_back(); }
 | T_const_char { $$ = new Constchar(constChars.back()); constChars.pop_back(); }
 | "(" r-value ")" { $$ = $2; }
 | "nil" { $$ = new NilR(); }
 | callr { $$ = $1; }
 | "@" expr { $$ = new Reference($2); }
 | "not" expr { $$ = new UnOp(operators.back(), $2); operators.pop_back(); }
 | "+" expr { $$ = new UnOp(operators.back(), $2); operators.pop_back(); }
 | "-" expr { $$ = new UnOp(operators.back(), $2); operators.pop_back(); }
 | expr "+" expr { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
 | expr "-" expr { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
 | expr "*" expr { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
 | expr "/" expr { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
 | expr "div" expr { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
 | expr "mod" expr { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
 | expr "or" expr { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
 | expr "and" expr { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
 | expr "=" expr { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
 | expr "<>" expr { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
 | expr "<" expr { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
 | expr "<=" expr { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
 | expr ">" expr { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
 | expr ">=" expr { $$ = new BinOp($1, operators.back(), $3); operators.pop_back(); }
 ;

call:
  T_id "(" expr  expr_list  ")" { $4->append_begin($3); $$ = new Call(ids.back(), $4); ids.pop_back(); }
  |T_id "(" ")" { $$ = new Call(ids.back()); ids.pop_back(); }
  ;

callr:
  T_id "(" expr  expr_list  ")" { $4->append_begin($3); $$ = new Callr(ids.back(), $4); ids.pop_back();}
  |T_id "(" ")" { $$ = new Callr(ids.back()); ids.pop_back();}
  ;

expr_list:
  /*nothing*/ { $$ = new Expr_list(); }
  | expr_list "," expr { $1->append_expr($3); $$ = $1; }
  ;



%%

int main() {

  int result = yyparse();
  bool first = true;
  if(DEBUGPARSER){
    std::cout << "\n\nRemaining ids: ";
    for (char* id : ids) {
      if (!first) std::cout << ", ";
      first = false;
      if(id) std::cout << id;
    }
    std::cout << "\n";
  }
  if (result == 0) printf("\nSuccess.\n");
  return result;
}
