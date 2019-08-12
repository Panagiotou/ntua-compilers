#include "../error.h"
#include "../lexer/lexer.hpp"
#include "symbol.hpp"
#include <cstring>
#include <iostream>
#include <vector>



inline std::ostream& operator<<(std::ostream &out, Types t) {
  switch (t) {
  case TYPE_INTEGER: out << "int"; break;
  case TYPE_BOOLEAN: out << "bool"; break;
  case TYPE_REAL: out << "real"; break;
  case TYPE_ARRAY: out << "array known length"; break;
  case TYPE_IARRAY: out << "array unknown length"; break;
  case TYPE_CHAR: out << "char"; break;
  case TYPE_STRING: out << "string"; break;
  case TYPE_POINTER: out << "pointer"; break;
  case TYPE_PROCEDURE: out << "type procedure"; break;
  }
  return out;
}



extern std::vector<int> rt_stack;

inline std::ostream& operator<<(std::ostream &out, const AST &t) {
  t.printOn(out);
  return out;
}

class Expr: public AST {
public:
  virtual int eval() const = 0;
  bool type_check(Type *t) {

    if (type == t) {
      return 1;
    }
    else{
      return 0;
    }
  }
  Type *getType(){
    return type;
  }
  Type *type;
};

class Expr_list: public AST{
public:
  Expr_list(): expr_list(){}
  ~Expr_list() {
    for (Expr *e : expr_list) delete e;
  }
  void append_expr(Expr *e) { expr_list.push_back(e); }
  void append_begin(Expr *e) { expr_list.insert(expr_list.begin(), e); }
  virtual void printOn(std::ostream &out) const override {
    out << "Expr_list(";
    bool first = true;
    for (Expr *e : expr_list) {
      if (!first) out << ", ";
      first = false;
      e->printOn(out);
    }
    out << ")";
  }
  std::vector<Expr *> getList(){
    return expr_list;
  }
  virtual void sem() override{
    for (Expr *e : expr_list) e->sem();
  }
private:
   std::vector<Expr *> expr_list;
};



class Rval: public Expr {
public:
  virtual int eval() const override {
    std::cout << "Evaluating Rval";
    return -1;
  }
};
class Lval: public Expr {
public:
  virtual int eval() const override {
    std::cout << "Evaluating Lval";
    return -1;
  }
};

class BinOp: public Rval {
public:
  BinOp(Expr *l, char *o, Expr *r): left(l), op(o), right(r) {
   }
  ~BinOp() { delete left; delete right; }
  virtual void printOn(std::ostream &out) const override {
    out << "BinOp(";
    left->printOn(out);
    if(op) out << op;
    right->printOn(out);
    out << ")";
  }
  virtual bool check_number(Expr *left, Expr *right){

    return ((left->type->val == TYPE_REAL) && (right->type->val == TYPE_INTEGER))||((left->type->val ==TYPE_INTEGER) && (right->type->val ==TYPE_REAL))||((left->type->val ==TYPE_REAL) && (right->type->val == TYPE_REAL))||((left->type->val == TYPE_INTEGER) && (right->type->val == TYPE_INTEGER));
  }
  virtual void sem() override {
    left->sem();
    right->sem();
    if(! strcmp(op, "+") || ! strcmp(op, "*") || ! strcmp(op, "-")){
      if( left->type->val == TYPE_INTEGER && right->type->val == TYPE_INTEGER){
        type = new Integer();
      }
      else if(left->type->val == TYPE_REAL || right->type->val == TYPE_REAL){
        type = new Real();
      }
      else{
        ERROR("Type mismatch!\n"); std::cout << left->type->val << op << right->type->val << "\n"; printOn(std::cout); exit(1);
      }
    }
    if(! strcmp(op, "/")){
      if(check_number(left, right)){
        type = new Real();
      }
      else{
        ERROR("Type mismatch!\n"); std::cout << left->type->val << op << right->type->val << "\n"; printOn(std::cout); exit(1);
      }
    }
    if(! strcmp(op, "mod") || ! strcmp(op, "div")){
      if( left->type->val == TYPE_INTEGER && right->type->val == TYPE_INTEGER){
        type = new Integer();
      }
      else{
        ERROR("Type mismatch!\n"); std::cout << left->type->val << op << right->type->val << "\n"; printOn(std::cout); exit(1);
      }
    }
    if(! strcmp(op, "=") || ! strcmp(op, "<>")){
      if(check_number(left, right) || ((left->type->val == right->type->val) && (left->type->val != TYPE_ARRAY || left->type->val != TYPE_IARRAY))){
        type = new Boolean();
      }
      else{
        ERROR("Type mismatch!\n"); std::cout << left->type->val << op << right->type->val << "\n"; printOn(std::cout); exit(1);
      }
    }
    if(! strcmp(op, "<") || ! strcmp(op, ">") || ! strcmp(op, "<=") || ! strcmp(op, ">=")){
      if(check_number(left, right)){
        type = new Boolean();
      }
      else{
        ERROR("Type mismatch!\n"); std::cout << left->type->val << op << right->type->val << "\n"; printOn(std::cout); exit(1);
      }
    }
    if(! strcmp(op, "or") || ! strcmp(op, "and")){
      if(left->type->val == TYPE_BOOLEAN && right->type->val == TYPE_BOOLEAN){
        type = new Boolean();
      }
      else{
        ERROR("Type mismatch!\n"); std::cout << left->type->val << op << right->type->val << "\n"; printOn(std::cout); exit(1);
      }
    }
  }
  virtual int eval() const override {
    if(! strcmp(op, "+")) return left->eval() + right->eval();
    if(! strcmp(op, "-")) return left->eval() - right->eval();
    if(! strcmp(op, "*")) return left->eval() * right->eval();
    if(! strcmp(op, "/")) return left->eval() / right->eval();
    if(! strcmp(op, "=")) return left->eval() == right->eval();
    if(! strcmp(op, "<")) return left->eval() < right->eval();
    if(! strcmp(op, ">")) return left->eval() > right->eval();
    if(! strcmp(op, "<=")) return left->eval() <= right->eval();
    if(! strcmp(op, ">=")) return left->eval() >= right->eval();
    if(! strcmp(op, "<>")) return left->eval() != right->eval();
    if(! strcmp(op, "div")) return left->eval() / right->eval();
    if(! strcmp(op, "mod")) return left->eval() % right->eval();
    if(! strcmp(op, "or")) return left->eval() || right->eval();
    if(! strcmp(op, "and")) return left->eval() && right->eval();
    return 0;  // this will never be reached.
  }
private:
  Expr *left;
  char *op;
  Expr *right;
};


class UnOp: public Rval {
public:
  UnOp(char *o, Expr *r): op(o), right(r) {}
  ~UnOp() { delete right; }
  virtual void printOn(std::ostream &out) const override {
    out << "UnOp(";
    if(op) out << op;
    right->printOn(out);
    out << ")";
  }
  virtual void sem() override {
    right->sem();
    if(! strcmp(op, "+") || ! strcmp(op, "-")){
      if(right->type->val == TYPE_INTEGER || right->type->val == TYPE_REAL){
        type = right->type;
      }
      else{
        ERROR("Type mismatch!\n"); std::cout << op << right->type->val << "\n"; exit(1);
      }
    }
    if( ! strcmp(op, "not")){
      if(right->type->val == TYPE_BOOLEAN){
        type = new Boolean();
      }
      else{
        ERROR("Type mismatch!\n"); std::cout << op << right->type->val << "\n"; exit(1);
      }
    }
  }
  virtual int eval() const override {
    if(! strcmp(op, "+")) return  right->eval();
    if(! strcmp(op, "-")) return -right->eval();
    if(! strcmp(op, "not")) return !right->eval();
    return 0;  // this will never be reached.
  }
private:
  char *op;
  Expr *right;
};

class Id: public Lval {
public:
  Id(char* v): var(v), offset(-1){   }
  virtual void printOn(std::ostream &out) const override {
    out << "Id(" << var << "@" << offset << ")";
  }
  virtual int eval() const override {
    return rt_stack[offset];
  }
  virtual void sem() override {
    // SymbolEntry *e = st.lookup(var);
    // Types type = e->type;
    // offset = e->offset;
    std::string s = var;
    type = st.lookup(s)->type;
  }
private:
  char* var;
  int offset;

};

class ArrayItem: public Lval {
public:
  ArrayItem(Lval *l, Expr *e){
    lval = l;
    expr = e;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "ArrayItem(";
    if(lval)lval->printOn(out);
    if(expr)expr->printOn(out);
    out << ")";
  }
  virtual int eval() const override {
    std::cout << "Evaluating ArrayItem";
    return -1;
  }
private:
  Lval *lval;
  Expr *expr;
};

class Reference: public Rval {
public:
  Reference(Lval *l){
    lval = l;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Reference(";
    if(lval)lval->printOn(out);
    out << ")";
  }
  virtual int eval() const override {
    std::cout << "Evaluating Reference";
    return -1;
  }
private:
  Lval *lval;
};




class Stmt: public AST {
public:
  virtual void run() const = 0;
};

class Dispose: public Stmt{
public:
  Dispose(Lval *l){
    lval = l;
    expr = nullptr;
  }
  Dispose(Expr *e){
    expr = e;
    lval = nullptr;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Dispose(";
    if(lval) lval->printOn(out);
    else expr->printOn(out);
    out << ")";
  }
  virtual void run() const override {
    std::cout << "Running Dispose";
  }
private:
  Lval *lval;
  Expr *expr;
};

class IdLabel: public Stmt{
public:
  IdLabel(char* i, Stmt *s){
    id = i;
    stmt = s;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "IdLabel(";
    if(id) out << id << " ";
    if(stmt) stmt->printOn(out);
    out << ")";
  }
  virtual void run() const override {
    std::cout << "Running IdLabel";
  }
private:
  char* id;
  Stmt *stmt;
};

class Assign: public Stmt{
public:
  Assign(Lval *l, Expr *e){
    lval = l;
    exprRight = e;
    exprPointer = nullptr;
  }
  Assign(Expr *e1, Expr *e2){
    exprPointer = e1;
    exprRight = e2;
    lval = nullptr;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Assign(";
    if(lval && exprRight){lval->printOn(out); out << " := "; exprRight->printOn(out);}
    if(exprPointer && exprRight){ exprPointer->printOn(out); out << " ^ := "; exprRight->printOn(out);}
    out << ")";
  }
  virtual void run() const override {
    std::cout << "Running Assign";
  }
  virtual void sem() override{
    if(lval && exprRight){
      if(lval->type != exprRight->type){
        std::cout << "Assign Type missmatch!"; exit(1);
      }
    }
    if(exprPointer && exprRight){
      if(exprPointer->type != exprRight->type){
        std::cout << "Assign Type missmatch!"; exit(1);
      }
    }
  }
private:
  Expr *exprPointer;
  Expr *exprRight;
  Lval *lval;
};

class Return: public Stmt{
public:
  Return(){};
  virtual void printOn(std::ostream &out) const override {
    out << "Return(";
    out << ")";
  }
  virtual void run() const override {
    std::cout << "Running Return";
  }
private:
};

class Id;

class Id_list: public AST{
public:
  Id_list(): id_list(){}
  ~Id_list() {
    for (char *id : id_list) delete id;
  }
  void append_id(char* id) { id_list.push_back(id); }
  void append_begin(char *i) { id_list.insert(id_list.begin(), i); }

  virtual void printOn(std::ostream &out) const override {
    out << "Id_list(";
    bool first = true;
    for (char* id : id_list) {
      if (!first) out << ", ";
      first = false;
      if(id) out << id << " ";
    }
    out << ")";
  }
  std::vector<char* > getlist(){
    return id_list;
  }
  int length(){
    return id_list.size();
  }
  std::vector<char* > charList(){
    return id_list;
  }
private:
   std::vector<char* > id_list;
};

class Formal: public AST {
public:
  Formal(Id_list *i_list, Type *t){
    id_list = i_list;
    type = t;
  }

  virtual void printOn(std::ostream &out) const override {
    out << "Formal(";
    id_list->printOn(out);
    type->printOn(out);
    out << ")";
 }
 virtual void sem() override{
   for (char *id : id_list->getlist()) {
     std::string var = id;

     st.insert(var, type);
   }
 }
 virtual void sem_() override{
   for (char *id : id_list->getlist()) {
     std::string var = id;
     var = "_" + var;
     st.insert(var, type);
   }
 }
 Type *getType(){
   return type;
 }
 std::vector<char* > getIdList(){
   return id_list->charList();
 }
private:
  Id_list *id_list;
  Type *type;
};

class Formal_list: public AST{
public:
  Formal_list(): formal_list(){}
  ~Formal_list() {
    for (Formal *f : formal_list) delete f;
  }
  void append_formal(Formal *f) { formal_list.push_back(f); }
  void append_begin(Formal *f) { formal_list.insert(formal_list.begin(), f); }

  virtual void printOn(std::ostream &out) const override {
    out << "Formal_list(";
    bool first = true;
    for (Formal *f : formal_list) {
      if (!first) out << ", ";
      first = false;
      f->printOn(out);
    }
    out << ")";
  }
  std::vector<Formal *> getList(){
    return formal_list;
  }
  virtual void sem() {
    for (Formal *f : formal_list) f->sem();
  }
  virtual void sem_() {
    for (Formal *f : formal_list) f->sem_();
  }
private:
   std::vector<Formal *> formal_list;
};

class Call: public Stmt{
public:
  Call(){
    id = nullptr;
    expr_list = nullptr;
  }
  Call(char* i, Expr_list *e = nullptr){
    id = i;
    expr_list = e;
  }
  ~Call(){
    delete id; delete expr_list;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Call(";
    if(id) out << id << " ";
    if(expr_list) expr_list->printOn(out);
    out << ")";
  }
  virtual void run() const override {
    std::cout << "Running Call";
  }
  virtual void sem() override {
    std::string s = id;
    expr_list->sem();
    SymbolEntry *se;
    se = st.lookup(s);
    s = se->s;
    if(st.isProcedure(s)){
      std::vector<Formal *> formal_list;
      formal_list = st.getFormalsProcedure(s)->getList();
      int i = 0;
      int argumentsExpected = 0;
      int argumentsProvided = 0;
      for (Formal *f : formal_list) {
        int FormalTimes;
        FormalTimes = f->getIdList().size();
        argumentsExpected += FormalTimes;
      }
      argumentsProvided = expr_list->getList().size();
      if(argumentsExpected != argumentsProvided){
        std::cout << "Procedure " << s << " expected " << argumentsExpected << " arguments " << " got " << argumentsProvided;
        exit(1);
      }
      for (Formal *f : formal_list) {
        int FormalTimes;
        FormalTimes = f->getIdList().size();
        for (int j=0; j<FormalTimes; j++){
          if(!(*f->getType() == *expr_list->getList().at(i)->getType())){
            ERROR("Type mismatch on procedure arguments!\n");
            std::cout << "In procedure "<< s << " arguments:\n";
            std::cout << f->getIdList().at(j);
            std::cout << "\n and \n";
            expr_list->getList().at(i)->printOn(std::cout);
            std::cout << "\nHave different types of ";
            f->getType()->printOn(std::cout);
            std::cout << " and ";
            expr_list->getList().at(i)->getType()->printOn(std::cout);
            exit(1);
          }
          i += 1;
        }
      }
    }
    else if(st.isFunction(s)){
      std::vector<Formal *> formal_list;
      formal_list = st.getFormalsFunction(s)->getList();
      int i = 0;
      int argumentsExpected = 0;
      int argumentsProvided = 0;
      for (Formal *f : formal_list) {
        int FormalTimes;
        FormalTimes = f->getIdList().size();
        argumentsExpected += FormalTimes;
      }
      argumentsProvided = expr_list->getList().size();
      if(argumentsExpected != argumentsProvided){
        std::cout << "Procedure " << s << " expected " << argumentsExpected << " arguments " << " got " << argumentsProvided;
        exit(1);
      }
      for (Formal *f : formal_list) {
        int FormalTimes;
        FormalTimes = f->getIdList().size();
        for (int j=0; j<FormalTimes; j++){
          if(!(*f->getType() == *expr_list->getList().at(i)->getType())){
            ERROR("Type mismatch on function arguments!\n");
            std::cout << "In function "<< s << " arguments:\n";
            std::cout << f->getIdList().at(j);
            std::cout << "\n and \n";
            expr_list->getList().at(i)->printOn(std::cout);
            std::cout << "\nHave different types of ";
            f->getType()->printOn(std::cout);
            std::cout << " and ";
            expr_list->getList().at(i)->getType()->printOn(std::cout);
            exit(1);
          }
          i += 1;
        }
      }
    }
  }

private:
  char* id;
  Expr_list *expr_list;
};

class Callr: public Rval{
public:
  Callr(){
    id = nullptr;
    expr_list = nullptr;
  }
  Callr(char *i, Expr_list *e = nullptr){
    id = i;
    expr_list = e;
  }
  ~Callr(){
    delete expr_list;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Callr(";
    if(id) out << id << " ";
    if(expr_list) expr_list->printOn(out);
    out << ")";
  }
private:
  char *id;
  Expr_list *expr_list;
};

class New: public Stmt{
public:
  New(Lval *l){
    lval = l;
    exprPointer = nullptr;
    exprBrackets = nullptr;
  }
  New(Expr *e){
    exprPointer = e;
    lval = nullptr;
    exprBrackets = nullptr;
  }
  New(Expr *e1, Expr *e2){
    exprBrackets = e1;
    exprPointer = e2;
    lval = nullptr;
  }
  New(Expr *e, Lval *l){
    exprBrackets = e;
    lval = l;
    exprPointer = nullptr;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "New(";
    if(lval) lval->printOn(out);
    if(exprPointer) exprPointer->printOn(out);
    if(exprBrackets) exprBrackets->printOn(out);
    out << ")";
  }
  virtual void run() const override {
    std::cout << "Running New";
  }
private:
  Lval *lval;
  Expr *exprPointer;
  Expr *exprBrackets;
};

class Goto: public Stmt{
public:
  Goto(char* i){
    id = i;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Goto(";
    out << id << " ";
    out << ")";
  }
  virtual void run() const override {
    std::cout << "Running Goto";
  }
private:
  char* id;
};


class Stmt_list: public AST{
public:
  Stmt_list(): stmt_list(){}
  ~Stmt_list() {
    for (Stmt *s : stmt_list) delete s;
  }
  void append_stmt(Stmt *s) { stmt_list.push_back(s); }
  void append_begin(Stmt *s) { stmt_list.insert(stmt_list.begin(), s); }
  virtual void printOn(std::ostream &out) const override {
    out << "\nStmt_list(";
    bool first = true;
    for (Stmt *s : stmt_list) {
      if (!first) out << ", ";
      first = false;
      out << "\n\t";
      s->printOn(out);
    }
    out << "\n)";
  }
  virtual void sem() override {
    for (Stmt *s : stmt_list) s->sem();
  }
private:
   std::vector<Stmt *> stmt_list;
};


class Constint: public Rval {
public:
  Constint(int c): con(c) {
    type = new Integer();
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Constint(";
    out << con ;
    out << ")";
  }
  virtual int eval() const override { return con; }
  // virtual void sem() override { type = new Integer(); }
  virtual int get(){
    return con;
  }
private:
  int con;
};



class Constchar: public Rval {
public:
  Constchar(char c): con(c) {
    type = new Char();}
  virtual void printOn(std::ostream &out) const override {
    out << "Constchar(" << con << ")";
  }
  virtual int eval() const override { return 0; } //wrong
  // virtual void sem() override { type = new Char(); }
private:
  char con;
};

class Conststring: public Lval {
public:
  Conststring(char *c): con(c) {
    type = new Array(new Char());}
  virtual void printOn(std::ostream &out) const override {
    out << "Conststring(" << con << ")";
  }
  virtual int eval() const override { return 0; } //wrong
  // virtual void sem() override { type = new String(); }
private:
  char *con;
};

class Constreal: public Rval {
public:
  Constreal(double c): con(c) {
    type = new Real();}
  virtual void printOn(std::ostream &out) const override {
    out << "Constreal(" << con << ")";
  }
  virtual int eval() const override { return 0; } //wrong
  // virtual void sem() override { type = new Real(); }
private:
  double con;
};

class If: public Stmt {
public:
  If(Expr *c, Stmt *s1, Stmt *s2 = nullptr):
    cond(c), stmt1(s1), stmt2(s2) {    }
  ~If() { delete cond; delete stmt1; delete stmt2; }
  virtual void printOn(std::ostream &out) const override {
    out << "If(";
    if(cond) cond->printOn(out);
    if(stmt1) stmt1->printOn(out);
    if (stmt2) stmt2->printOn(out);
    out << ")";
  }
  virtual void sem() override {
    cond->sem();
    if(cond->type->val == TYPE_BOOLEAN){
      stmt1->sem();
      if (stmt2 != nullptr) stmt2->sem();
    }
    else{
      ERROR("Type mismatch, cond is not bool!\n"); printOn(std::cout); exit(1);
    }
  }
  virtual void run() const override {
    if (cond->eval())
      stmt1->run();
    else if (stmt2 != nullptr)
      stmt2->run();
  }
private:
  Expr *cond;
  Stmt *stmt1;
  Stmt *stmt2;
};

class While: public Stmt {
public:
  While(Expr *e, Stmt *s): expr(e), stmt(s) { }
  ~While() { delete expr; delete stmt; }
  virtual void printOn(std::ostream &out) const override {
    out << "While(" << *expr << ", " << *stmt << ")";
  }
  virtual void sem() override {
    expr->sem();
    if(expr->type->val == TYPE_BOOLEAN){
      stmt->sem();
    }
    else{
      ERROR("Type mismatch!\n"); exit(1);
    }
  }
  virtual void run() const override {
    while(expr->eval()){
      stmt->run();
    }
  }
private:
  Expr *expr;
  Stmt *stmt;
};


class Block: public Stmt{
public:
  Block(Stmt_list *s = nullptr){
    if(s) stmt_list = s;
  }
  ~Block(){
    delete stmt_list;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Block(";
    if(stmt_list) stmt_list->printOn(out);
    out << ")";
  }
  virtual void sem() override {
    stmt_list->sem();
  }
virtual void run() const override {
  std::cout << "Running block";
}
private:
  Stmt_list *stmt_list;
};

class Header: public AST{
public:
  virtual char *getFunctionName(){return nullptr;};
};




class Label: public AST{
public:
  Label(Id_list *i_l){
    id_list = i_l;
  };
  ~Label(){
    delete id_list;
  };
  virtual void printOn(std::ostream &out) const override {
    out << "Label(";
    id_list->printOn(out);
    out << ")";
  };
  virtual void sem() override {
    id_list->sem();
  }
private:
  Id_list *id_list;
};

class Decl: public AST {
public:
  Decl(Id_list *i_list, Type *t){
    id_list = i_list;
    type = t;
  }

  virtual void printOn(std::ostream &out) const override {
    out << "Decl(";
    id_list->printOn(out);
    type->printOn(out);
    out << ")";
  }
  virtual void sem() override{
    for (char *id : id_list->getlist()) {
      std::string var = id;
      st.insert(var, type);
    }
  }
private:
  Id_list *id_list;
  Type *type;
};

class Decl_list: public AST{
public:
  Decl_list(): decl_list(){}
  ~Decl_list() {
    for (Decl *d : decl_list) delete d;
  }
  void append_decl(Decl *d) { decl_list.push_back(d); }
  void append_begin(Decl *d) { decl_list.insert(decl_list.begin(), d); }

  virtual void printOn(std::ostream &out) const override {
    out << "Decl_list(";
    bool first = true;
    for (Decl *d : decl_list) {
      if (!first) out << ", ";
      first = false;
      d->printOn(out);
    }
    out << ")";
  }
  virtual void sem() override {
    for (Decl *d : decl_list) d->sem();
  }
private:
   std::vector<Decl *> decl_list;
};




class Procedure: public Header{
public:
  Procedure(char* i, Formal_list *f = nullptr){
    id = i;
    formal_list = f;
  }
  ~Procedure(){
    delete id; delete formal_list;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Procedure(";
    if(id) out << id << " ";
    formal_list->printOn(out);
    out << ")";
  }
  virtual void sem() override {
    std::string s = id;
    st.insertProcedure(s, new ProcedureType(), formal_list);
    formal_list->sem();
  }
  virtual void sem_() override {
    std::string s = id;
    s = "_" + s;
    st.insertProcedure(s, new ProcedureType(), formal_list);
    formal_list->sem_();
  }
private:
  char* id;
  Formal_list *formal_list;
};

class Function: public Header{
public:
  Function(char* i, Type *t, Formal_list *f = nullptr){
    id = i;
    type = t;
    formal_list = f;
  }
  ~Function(){
    delete id; delete formal_list;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Function(";
    out << id << " ";
    formal_list->printOn(out);
    type->printOn(out);
    out << ")";
  }
  virtual void sem() override {
    std::string s = id;
    st.insertFunction(s, type, formal_list);
    formal_list->sem();
  }
  virtual void sem_() override {
    std::string s = id;
    s = "_" + s;
    st.insertFunction(s, type, formal_list);
    formal_list->sem_();
  }
  virtual char *getFunctionName () override{
    return id;
  }
private:
  char* id;
  Type *type;
  Formal_list *formal_list;
};

class Body;

class Local: public AST{
public:
  Local(Decl_list *d){
    // "var"
    decl_list = d;
    localType = "var";
  };
  Local(Label *l){
    // label
    label = l;
    localType = "label";
  };
  Local(Header *h, AST *b){
    // function or procedure
    header = h;
    body = b;
    localType = "forp";
  };
  Local(Header *h){
    // forward
    header = h;
    localType = "forward";
  };
  ~Local(){};
  virtual void printOn(std::ostream &out) const override {
    out << "Local(";
    if(localType.compare("var") == 0){
      decl_list->printOn(out);
    }
    else if(localType.compare("label") == 0){
      label->printOn(out);
    }
    else if(localType.compare("forp") == 0){
      header->printOn(out);
      body->printOn(out);
    }
    else if(localType.compare("forward") == 0){
      header->printOn(out);
    }
    out << ")";
  };
  virtual void sem() override {
    if(localType.compare("var") == 0){
      decl_list->sem();
    }
    else if(localType.compare("label") == 0){
      label->sem();
    }
    else if(localType.compare("forp") == 0){
      header->sem();
      body->semPorF(header, header->getFunctionName());
    }
    else if(localType.compare("forward") == 0){
      header->sem();
    }
  }
  char *getFunctionName(){
    return header->getFunctionName();
  }
private:
  Decl_list *decl_list;
  Label *label;
  Header *header;
  AST *body;
  std::string localType;
};

class Local_list: public AST{
public:
  Local_list(): local_list(){}
  ~Local_list() {
    for (Local *l : local_list) delete l;
  }
  void append_local(Local *l) { local_list.push_back(l); }
  void append_begin(Local *l) { local_list.insert(local_list.begin(), l); }

  virtual void printOn(std::ostream &out) const override {
    out << "Local_list(";
    bool first = true;
    for (Local *l : local_list) {
      if (!first) out << ", ";
      first = false;
      l->printOn(out);
    }
    out << ")\n";
  }
  virtual void sem() override {
    for (Local *l : local_list) l->sem();
  }
private:
   std::vector<Local *> local_list;
};

class Body: public AST{
public:
  Body(Local_list *l, Block *b){
    local_list = l;
    block = b;
  }
  ~Body() {
    delete local_list;
    delete block;
  }
  virtual void sem() override {
    st.openScope();
    local_list->sem();
    block->sem();
    st.closeScope();
  }
  void semPorF( AST *h, char *funName = nullptr) override {
    st.openScope();
    h->sem_();
    local_list->sem();
    block->sem();
    if(st.existsResult() && funName){
        std::cout << "Function " << funName << " does not return\n";
        exit(1);
    }
    st.closeScope();
  }
  void merge(Block *b) {
    block = b;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Body(";
    if(local_list) local_list->printOn(out);
    if(block) block->printOn(out);
    out << ")";
  }
private:
  Local_list *local_list;
  Block *block;
};
